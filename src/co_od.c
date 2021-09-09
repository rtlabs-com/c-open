/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2017 rt-labs AB, Sweden.
 *
 * This software is dual-licensed under GPLv3 and a commercial
 * license. See the file LICENSE.md distributed with this software for
 * full license information.
 ********************************************************************/

#ifdef UNIT_TEST
#endif

#include "co_od.h"
#include "co_sdo.h"
#include "co_util.h"

#include <string.h>
#include <inttypes.h>

#define OD_RESTORE 0x64616F6C /* Signature for restore ("load") */
#define OD_STORE   0x65766173 /* Signature for store ("save") */

static int co_subindex_equals (
   co_net_t * net,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uintptr_t arg)
{
   uint8_t subindex = (uint8_t)arg;
   return entry->subindex == subindex;
}

static void co_od_notify (
   co_net_t * net,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex)
{
   if (entry->flags & OD_NOTIFY)
   {
      if (net->cb_notify)
         net->cb_notify (net, obj->index, subindex);
   }
}

const co_entry_t * co_entry_find (
   co_net_t * net,
   const co_obj_t * obj,
   uint8_t subindex)
{
   CC_ASSERT (obj->entries != NULL);

   /* Subindex 0 always exists */
   if (subindex == 0)
   {
      return &obj->entries[0];
   }

   /* VAR objects have no other subindexes */
   if (obj->objtype == OTYPE_VAR)
   {
      return NULL;
   }

   /* ARRAY and RECORD objects may have ARRAY subindexes */
   if (obj->objtype == OTYPE_ARRAY || obj->objtype == OTYPE_RECORD)
   {
      if (obj->entries[1].flags & OD_ARRAY)
      {
         return (subindex <= obj->max_subindex) ? &obj->entries[1] : NULL;
      }
   }

   /* Otherwise search descriptor for matching subindex */
   return co_obj_traverse (net, obj, co_subindex_equals, subindex, obj->max_subindex);
}

const co_obj_t * co_obj_find (co_net_t * net, uint16_t index)
{
   const co_obj_t * obj = net->od;

   /* Walk table until it ends or index is found */
   while (obj->index != 0 && obj->index != index)
   {
      obj++;
   }

   /* Return object if found */
   if (obj->index != 0)
      return obj;

   /* Object was not found */
   return NULL;
}

uint32_t co_od_get_ptr (
   co_net_t * net,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint8_t ** ptr)
{
   if (obj->access)
   {
      /* Access function has no storage */
      return CO_SDO_ABORT_GENERAL;
   }

   /* Get pointer to storage */
   *ptr = (uint8_t *)entry->data;
   if (*ptr == NULL)
   {
      /* Get pointer to constant value */
      *ptr = (uint8_t *)&entry->value;
   }
   else if (entry->flags & OD_ARRAY)
   {
      /* Get pointer to array member */
      *ptr += (subindex - 1) * CO_BYTELENGTH (entry->bitlength);
   }
   return 0;
}

uint32_t co_od_get_value (
   co_net_t * net,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint64_t * value)
{
   uint32_t abort;
   uint8_t * data;

   if (obj->access)
   {
      uint32_t v;

      /* Call object access function. For subindex 0, function may
         return BAD_SUBINDEX to indicate that it did not handle the
         access. */
      abort = obj->access (net, OD_EVENT_READ, obj, entry, subindex, &v);
      if (!(subindex == 0 && abort == CO_SDO_ABORT_BAD_SUBINDEX))
      {
         *value = v;
         return abort;
      }
   }

   /* Get pointer to storage */
   data = (uint8_t *)entry->data;
   if (data == NULL)
   {
      /* Get pointer to constant value */
      data = (uint8_t *)&entry->value;
   }
   else if (entry->flags & OD_ARRAY)
   {
      /* Get pointer to array member */
      data += (subindex - 1) * CO_BYTELENGTH (entry->bitlength);
   }

   switch (entry->datatype)
   {
   case DTYPE_BOOLEAN:
   case DTYPE_UNSIGNED8:
   case DTYPE_INTEGER8:
      *value = co_atomic_get_uint8 (data);
      break;

   case DTYPE_UNSIGNED16:
   case DTYPE_INTEGER16:
      *value = co_atomic_get_uint16 (data);
      break;

   case DTYPE_REAL32:
   case DTYPE_UNSIGNED32:
   case DTYPE_INTEGER32:
      *value = co_atomic_get_uint32 (data);
      break;

   case DTYPE_REAL64:
   case DTYPE_UNSIGNED64:
   case DTYPE_INTEGER64:
      *value = co_atomic_get_uint64 (data);
      break;

   case DTYPE_VISIBLE_STRING:
   case DTYPE_OCTET_STRING:
   case DTYPE_UNICODE_STRING:
      /* Get first 64 bits */
      *value = co_fetch_uint64 (data);
      break;

   default:
      CC_ASSERT (0);
   }

   return 0;
}

uint32_t co_od_set_value (
   co_net_t * net,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint64_t value)
{
   uint8_t * data;

   LOG_DEBUG (CO_OD_LOG, "set %x:%x = %" PRIx64 "\n", obj->index, subindex, value);

   if (obj->access)
   {
      uint32_t result;
      uint32_t v = value;

      result = obj->access (net, OD_EVENT_WRITE, obj, entry, subindex, &v);
      co_od_notify (net, obj, entry, subindex);
      return result;
   }

   data = entry->data;

   if (entry->flags & OD_ARRAY)
   {
      data += (subindex - 1) * CO_BYTELENGTH (entry->bitlength);
   }

   /* Set value in dictionary */
   switch (entry->datatype)
   {
   case DTYPE_BOOLEAN:
   case DTYPE_UNSIGNED8:
   case DTYPE_INTEGER8:
      co_atomic_set_uint8 (data, value & UINT8_MAX);
      break;

   case DTYPE_UNSIGNED16:
   case DTYPE_INTEGER16:
      co_atomic_set_uint16 (data, value & UINT16_MAX);
      break;

   case DTYPE_REAL32:
   case DTYPE_UNSIGNED32:
   case DTYPE_INTEGER32:
      co_atomic_set_uint32 (data, value & UINT32_MAX);
      break;

   case DTYPE_REAL64:
   case DTYPE_UNSIGNED64:
   case DTYPE_INTEGER64:
      co_atomic_set_uint64 (data, value);
      break;

   default:
      return CO_SDO_ABORT_GENERAL;
   }

   co_od_notify (net, obj, entry, subindex);
   return 0;
}

void co_od_set_defaults (co_net_t * net, uint16_t min, uint16_t max)
{
   const co_default_t * item = net->defaults;
   const co_obj_t * obj;
   const co_entry_t * entry;

   if (item == NULL)
      return;

   for (item = net->defaults; item->index != 0; item++)
   {
      uint32_t abort;

      if (item->index < min || item->index > max)
         continue;

      obj = co_obj_find (net, item->index);
      if (obj == NULL)
      {
         /* Not found in this dictionary, ignore */
         continue;
      }

      entry = co_entry_find (net, obj, item->subindex);
      if (entry == NULL)
      {
         LOG_WARNING (
            CO_OD_LOG,
            "bad subindex %x:%x\n",
            item->index,
            item->subindex);
         continue;
      }

      abort = co_od_set_value (net, obj, entry, item->subindex, item->value);
      if (abort)
      {
         LOG_WARNING (
            CO_OD_LOG,
            "abort restoring %x:%x\n",
            item->index,
            item->subindex);
      }
   }
}

void co_od_zero (co_net_t * net, uint16_t min, uint16_t max)
{
   const co_obj_t * obj;

   for (obj = net->od; obj->index != 0; obj++)
   {
      if (obj->index < min || obj->index > max)
         continue;

      if (obj->access)
      {
         obj->access (net, OD_EVENT_RESTORE, obj, NULL, 0, NULL);
      }
      else
      {
         const co_entry_t * entry = obj->entries;
         uint8_t subindex;

         do
         {
            if (entry->flags & OD_WRITE)
            {
               size_t size = CO_BYTELENGTH (entry->bitlength);
               if (entry->flags & OD_ARRAY)
               {
                  size = size * obj->max_subindex;
               }
               memset (entry->data, 0, size);
            }
            if (entry->flags & OD_ARRAY)
               break;
            subindex = entry->subindex;
            entry++;
         } while (subindex < obj->max_subindex);
      }
   }
}

uint32_t co_od_load (co_net_t * net, co_store_t store)
{
   const co_obj_t * obj;
   void * arg;
   size_t entries;

   if (net->open == NULL || net->read == NULL || net->close == NULL)
      return CO_SDO_ABORT_GENERAL;

   arg = net->open (store, CO_MODE_READ);
   if (arg == NULL)
      return CO_SDO_ABORT_GENERAL;

   /* Get number of entries */
   if (net->read (arg, &entries, sizeof (entries)) < 0)
      goto error;

   /* Load entries */
   while (entries-- > 0)
   {
      const co_entry_t * entry;
      uint16_t index;
      uint8_t subindex;
      size_t size;
      uint64_t value = 0;
      uint8_t * ptr;
      uint32_t abort;

      if (net->read (arg, &index, sizeof (index)) < 0)
         goto error;

      if (net->read (arg, &subindex, sizeof (subindex)) < 0)
         goto error;

      if (net->read (arg, &size, sizeof (size)) < 0 || size == 0)
         goto error;

      /* Attempt to set value. Errors are ignored to support firmware
         update of dictionary */

      obj = co_obj_find (net, index);
      if (obj == NULL)
         goto skip;

      entry = co_entry_find (net, obj, subindex);
      if (entry == NULL || !(entry->flags & OD_WRITE) || (entry->flags & OD_TRANSIENT))
         goto skip; /* Not storable in this OD */

      if (size <= sizeof (value))
      {
         if (net->read (arg, &value, size) < 0)
            goto error;

         co_od_set_value (net, obj, entry, subindex, value);
      }
      else if (size == CO_BYTELENGTH (entry->bitlength))
      {
         /* Get pointer to storage */
         abort = co_od_get_ptr (net, obj, entry, subindex, &ptr);
         if (abort)
            goto error;

         if (net->read (arg, ptr, size) < 0)
            goto error;
      }
      else
      {
         /* Stored size does not match object size. Discard data. */
         goto skip;
      }

      continue;
   skip:
      while (size > sizeof (value))
      {
         if (net->read (arg, &value, sizeof (value)) < 0)
            goto error;
         size -= sizeof (value);
      }

      if (net->read (arg, &value, size) < 0)
         goto error;
   }

   /* Ignore any error on close */
   net->close (arg);
   return 0;

error:
   net->close (arg);
   LOG_ERROR (CO_OD_LOG, "Failed to load OD\n");
   return CO_SDO_ABORT_GENERAL;
}

void co_od_reset (co_net_t * net, co_store_t store, uint16_t min, uint16_t max)
{
   co_od_zero (net, min, max);
   co_od_set_defaults (net, min, max);
   co_od_load (net, store);
}

uint32_t co_od_store (co_net_t * net, co_store_t store, uint16_t min, uint16_t max)
{
   const co_obj_t * obj;
   uint8_t subindex;
   void * arg;
   size_t entries = 0;

   if (net->open == NULL || net->write == NULL || net->close == NULL)
      return CO_SDO_ABORT_HW_ERROR;

   arg = net->open (store, CO_MODE_WRITE);
   if (arg == NULL)
      return CO_SDO_ABORT_HW_ERROR;

   /* Compute number of entries */
   for (obj = net->od; obj->index != 0; obj++)
   {
      if (obj->index < min || obj->index > max)
         continue;

      for (subindex = 0; subindex <= obj->max_subindex; subindex++)
      {
         const co_entry_t * entry;
         entry = co_entry_find (net, obj, subindex);
         if (entry != NULL && (entry->flags & OD_WRITE) && !(entry->flags & OD_TRANSIENT))
            entries++;
      }
   }

   /* Store number of entries */
   if (net->write (arg, &entries, sizeof (entries)) < 0)
      goto error;

   /* Store entries */
   for (obj = net->od; obj->index != 0; obj++)
   {
      if (obj->index < min || obj->index > max)
         continue;

      for (subindex = 0; subindex <= obj->max_subindex; subindex++)
      {
         const co_entry_t * entry;

         entry = co_entry_find (net, obj, subindex);
         if (entry != NULL && (entry->flags & OD_WRITE) && !(entry->flags & OD_TRANSIENT))
         {
            size_t size = CO_BYTELENGTH (entry->bitlength);
            uint64_t value;
            uint8_t * ptr;
            uint32_t abort;

            /* Write index */
            if (net->write (arg, &obj->index, sizeof (obj->index)) < 0)
               goto error;

            /* Write subindex */
            if (net->write (arg, &subindex, sizeof (subindex)) < 0)
               goto error;

            /* Write size of entry */
            if (net->write (arg, &size, sizeof (size)) < 0)
               goto error;

            if (size > sizeof (value))
            {
               /* Get pointer to storage */
               abort = co_od_get_ptr (net, obj, entry, subindex, &ptr);
               if (abort)
                  goto error;

               if (net->write (arg, ptr, size) < 0)
                  goto error;
            }
            else
            {
               /* Get value */
               abort = co_od_get_value (net, obj, entry, subindex, &value);
               if (abort)
                  goto error;

               if (net->write (arg, &value, size) < 0)
                  goto error;
            }
         }
      }
   }

   /* Finalize write */
   if (net->close (arg) < 0)
      return CO_SDO_ABORT_HW_ERROR;

   return 0;

error:
   /* Ignore any error on close */
   net->close (arg);
   LOG_ERROR (CO_OD_LOG, "Failed to store OD\n");
   return CO_SDO_ABORT_HW_ERROR;
}

uint32_t co_od_restore (co_net_t * net, co_store_t store)
{
   void * arg;
   size_t entries = 0;

   if (net->open == NULL || net->write == NULL || net->close == NULL)
      return CO_SDO_ABORT_HW_ERROR;

   arg = net->open (store, CO_MODE_WRITE);
   if (arg == NULL)
      return CO_SDO_ABORT_HW_ERROR;

   /* Clear entries */
   if (net->write (arg, &entries, sizeof (entries)) < 0)
      goto error;

   /* Finalize write */
   if (net->close (arg) < 0)
      return CO_SDO_ABORT_HW_ERROR;

   return 0;

error:
   /* Ignore any error on close */
   net->close (arg);
   LOG_ERROR (CO_OD_LOG, "Failed to restore OD\n");
   return CO_SDO_ABORT_HW_ERROR;
}

uint32_t co_od1010_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   uint32_t abort;

   if (subindex == 0 || subindex > obj->max_subindex)
      return CO_SDO_ABORT_BAD_SUBINDEX;

   switch (event)
   {
   case OD_EVENT_READ:
      /* Device saves parameters on command only */
      *value = BIT (0);
      return 0;
   case OD_EVENT_WRITE:
      if (*value == OD_STORE)
      {
         switch (subindex)
         {
         case 1:
            /* Store communication related parameters */
            abort = co_od_store (net, CO_STORE_COMM, 0x1000, 0x1FFF);
            if (abort)
               return abort;

            /* Store manufacturer-specific area */
            abort = co_od_store (net, CO_STORE_MFG, 0x2000, 0x5FFF);
            if (abort)
               return abort;

            /* Store application related parameters */
            abort = co_od_store (net, CO_STORE_APP, 0x6000, 0x9FFF);
            if (abort)
               return abort;

            return 0;

         case 2:
            /* Store communication related parameters */
            return co_od_store (net, CO_STORE_COMM, 0x1000, 0x1FFF);

         case 3:
            /* Store application related parameters */
            return co_od_store (net, CO_STORE_APP, 0x6000, 0x9FFF);

         case 4:
            /* Store manufacturer-specific area */
            return co_od_store (net, CO_STORE_MFG, 0x2000, 0x5FFF);
         }
      }
      return CO_SDO_ABORT_WRITE;
   default:
      return CO_SDO_ABORT_GENERAL;
   }
}

uint32_t co_od1011_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   uint32_t abort;

   if (subindex == 0 || subindex > obj->max_subindex)
      return CO_SDO_ABORT_BAD_SUBINDEX;

   switch (event)
   {
   case OD_EVENT_READ:
      *value = BIT (0);
      return 0;
   case OD_EVENT_WRITE:
      if (*value == OD_RESTORE)
      {
         switch (subindex)
         {
         case 1:
            /* Restore communication related parameters */
            abort = co_od_restore (net, CO_STORE_COMM);
            if (abort)
               return abort;

            /* Restore manufacturer-specific area */
            abort = co_od_restore (net, CO_STORE_MFG);
            if (abort)
               return abort;

            /* Restore application related parameters */
            abort = co_od_restore (net, CO_STORE_APP);
            if (abort)
               return abort;

            return 0;

         case 2:
            /* Restore communication related parameters */
            return co_od_restore (net, CO_STORE_COMM);

         case 3:
            /* Restore application related parameters */
            return co_od_restore (net, CO_STORE_APP);

         case 4:
            /* Restore application manufacturer-specific area */
            return co_od_restore (net, CO_STORE_MFG);
         }
      }
      return CO_SDO_ABORT_WRITE;
   default:
      return CO_SDO_ABORT_GENERAL;
   }
}

uint32_t co_od1020_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (subindex == 0 || subindex > obj->max_subindex)
      return CO_SDO_ABORT_BAD_SUBINDEX;

   switch (event)
   {
   case OD_EVENT_READ:
      switch (subindex)
      {
      case 1:
         *value = (net->config_dirty) ? 0 : net->config_date;
         break;
      case 2:
         *value = (net->config_dirty) ? 0 : net->config_time;
         break;
      }
      return 0;
   case OD_EVENT_WRITE:
      switch (subindex)
      {
      case 1:
         net->config_date = *value;
         break;
      case 2:
         net->config_time = *value;
         break;
      }
      return 0;
   default:
      return CO_SDO_ABORT_GENERAL;
   }
}
