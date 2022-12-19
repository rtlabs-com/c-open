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
#define os_channel_send        mock_os_channel_send
#define os_get_current_time_us mock_os_get_current_time_us
#define co_obj_find            mock_co_obj_find
#define co_entry_find          mock_co_entry_find
#define co_emcy_tx             mock_co_emcy_tx
#endif

#include "co_pdo.h"
#include "co_od.h"
#include "co_util.h"
#include "co_sdo.h"
#include "co_emcy.h"

#include <string.h>

#define CO_PDO_RTR BIT (30)

#define CO_PDO_TT_ACYCLIC    0x00
#define CO_PDO_TT_CYCLIC_MIN 0x01
#define CO_PDO_TT_CYCLIC_MAX 0xF0
#define CO_PDO_TT_RTR_SYNC   0xFC
#define CO_PDO_TT_RTR_EVENT  0xFD
#define CO_PDO_TT_EVENT_MF   0xFE
#define CO_PDO_TT_EVENT_PF   0xFF

#define IS_ACYCLIC(tt) ((tt) == CO_PDO_TT_ACYCLIC)

#define IS_CYCLIC(tt)                                                          \
   ((tt) >= CO_PDO_TT_CYCLIC_MIN && (tt) <= CO_PDO_TT_CYCLIC_MAX)

#define IS_RTR(tt) ((tt) == CO_PDO_TT_RTR_SYNC || (tt) == CO_PDO_TT_RTR_EVENT)

#define IS_EVENT(tt) ((tt) >= CO_PDO_TT_EVENT_MF)

#define VALIDATE_TT(tt, is_rx)                                                 \
   (IS_ACYCLIC (tt) || IS_CYCLIC (tt) || IS_EVENT (tt) ||                      \
    (!is_rx & IS_RTR (tt)))

static uint64_t bitslice_get (uint64_t * data, int offset, int length)
{
   const uint64_t mask = (length == 64) ? UINT64_MAX : (1ULL << length) - 1;

   CC_ASSERT (offset < 64);
   CC_ASSERT (length <= 64);

   return (*data >> offset) & mask;
}

static void bitslice_set (uint64_t * data, int offset, int length, uint64_t value)
{
   uint64_t mask = (length == 64) ? UINT64_MAX : (1ULL << length) - 1;

   CC_ASSERT (offset < 64);
   CC_ASSERT (length <= 64);

   mask  = mask << offset;
   *data = (*data & ~mask) | (value << offset);
}

void co_pdo_pack (co_net_t * net, co_pdo_t * pdo)
{
   unsigned int ix;
   unsigned int offset = 0;

   for (ix = 0; ix < pdo->number_of_mappings; ix++)
   {
      const co_entry_t * entry = pdo->entries[ix];
      const co_obj_t * obj     = pdo->objs[ix];
      size_t bitlength         = pdo->mappings[ix] & 0xFF;
      uint64_t value           = 0;

      if (entry != NULL)
      {
         co_od_get_value (net, obj, entry, entry->subindex, &value);
      }

      bitslice_set (&pdo->frame, offset, bitlength, value);
      offset += bitlength;
   }
}

void co_pdo_unpack (co_net_t * net, co_pdo_t * pdo)
{
   unsigned int ix;
   unsigned int offset = 0;

   for (ix = 0; ix < pdo->number_of_mappings; ix++)
   {
      const co_entry_t * entry = pdo->entries[ix];
      const co_obj_t * obj     = pdo->objs[ix];
      size_t bitlength         = pdo->mappings[ix] & 0xFF;
      uint64_t value;

      if (entry != NULL)
      {
         value = bitslice_get (&pdo->frame, offset, bitlength);
         co_od_set_value (net, obj, entry, entry->subindex, value);
      }

      offset += bitlength;
   }
}

static uint32_t co_pdo_mapping_validate (co_pdo_t * pdo, uint8_t number_of_mappings)
{
   int ix;

   /* Mappings array bounds check */
   if (number_of_mappings > MAX_PDO_ENTRIES)
      return CO_SDO_ABORT_PDO_LENGTH;

   /* Check that bitlength is OK */
   pdo->bitlength = 0;
   for (ix = 0; ix < number_of_mappings; ix++)
   {
      pdo->bitlength += pdo->mappings[ix] & 0xFF;
   }

   /* Must fit in single frame */
   if (pdo->bitlength > 64)
      return CO_SDO_ABORT_PDO_LENGTH;

   if (IS_CYCLIC (pdo->sync_start))
      pdo->sync_wait = true;

   return 0;
}

static uint32_t co_pdo_comm_get (
   co_net_t * net,
   co_pdo_t * pdo,
   uint8_t subindex,
   uint32_t * value)
{
   switch (subindex)
   {
   case 1:
      *value = pdo->cobid;
      break;
   case 2:
      *value = pdo->transmission_type;
      break;
   case 3:
      *value = pdo->inhibit_time;
      break;
   case 5:
      *value = pdo->event_timer;
      break;
   case 6:
      *value = pdo->sync_start;
      break;
   default:
      return CO_SDO_ABORT_BAD_SUBINDEX;
   }
   return 0;
}

static uint32_t co_pdo_comm_set (
   co_net_t * net,
   co_pdo_t * pdo,
   uint8_t subindex,
   uint32_t * value,
   bool is_rx)
{
   switch (subindex)
   {
   case 1:
   {
      if (!co_validate_cob_id (*value))
         return CO_SDO_ABORT_VALUE;
      if (((pdo->cobid | *value) & CO_COBID_INVALID) == 0 && net->state != STATE_INIT)
         return CO_SDO_ABORT_VALUE;
      pdo->cobid        = *value;
      pdo->sync_counter = 0;
      pdo->queued       = false;
      break;
   }
   case 2:
      if (!VALIDATE_TT (*value & 0xFF, is_rx))
         return CO_SDO_ABORT_VALUE;
      pdo->transmission_type = *value & 0xFF;
      break;
   case 3:
      if ((pdo->cobid & CO_COBID_INVALID) == 0 && net->state != STATE_INIT)
         return CO_SDO_ABORT_VALUE;
      pdo->inhibit_time = *value & 0xFFFF;
      break;
   case 5:
      pdo->event_timer = *value & 0xFFFF;
      break;
   case 6:
      if (is_rx)
         return CO_SDO_ABORT_BAD_SUBINDEX;
      if ((pdo->cobid & CO_COBID_INVALID) == 0 && net->state != STATE_INIT)
         return CO_SDO_ABORT_VALUE;
      pdo->sync_start = *value & 0xFF;
      break;
   default:
      return CO_SDO_ABORT_BAD_SUBINDEX;
   }
   return 0;
}

static uint32_t co_pdo_map_get (
   co_net_t * net,
   co_pdo_t * pdo,
   uint8_t subindex,
   uint32_t * value)
{
   if (subindex == 0)
      *value = pdo->number_of_mappings;
   else if (subindex > MAX_PDO_ENTRIES)
      return CO_SDO_ABORT_BAD_SUBINDEX;
   else
      *value = pdo->mappings[subindex - 1];

   return 0;
}

static uint32_t co_pdo_map_set (
   co_net_t * net,
   co_pdo_t * pdo,
   uint8_t subindex,
   uint32_t * value,
   bool is_rx)
{
   uint16_t mapped_index    = *value >> 16;
   uint8_t mapped_subindex  = (*value >> 8) & 0xFF;
   uint8_t mapped_bitlength = *value & 0xFF;
   const co_obj_t * obj;
   const co_entry_t * entry;

   if (net->state != STATE_INIT)
   {
      /* Must not change mapping while PDO is valid */
      if ((pdo->cobid & CO_COBID_INVALID) != CO_COBID_INVALID)
         return CO_SDO_ABORT_GENERAL;
   }

   if (subindex == 0)
   {
      uint8_t number_of_mappings = *value & 0xFF;

      uint32_t abort = co_pdo_mapping_validate (pdo, number_of_mappings);
      if (abort)
         return abort;

      /* Update number of mappings */
      pdo->number_of_mappings = number_of_mappings;

      return 0;
   }
   else if (subindex > MAX_PDO_ENTRIES)
   {
      return CO_SDO_ABORT_BAD_SUBINDEX;
   }

   /* Check that number_of_mappings (subindex 0) is zero before write */
   if ((pdo->number_of_mappings != 0) && (net->state != STATE_INIT))
   {
      return CO_SDO_ABORT_ACCESS;
   }

   /* Check for padding */
   if (co_is_padding (mapped_index, mapped_subindex))
   {
      pdo->mappings[subindex - 1] = *value;
      pdo->objs[subindex - 1]     = NULL;
      pdo->entries[subindex - 1]  = NULL;
      return 0;
   }

   /* Find mapped object */
   obj = co_obj_find (net, mapped_index);
   if (obj == NULL)
   {
      return CO_SDO_ABORT_BAD_INDEX;
   }

   /* Find mapped entry */
   entry = co_entry_find (net, obj, mapped_subindex);
   if (entry == NULL)
   {
      return CO_SDO_ABORT_BAD_SUBINDEX;
   }

   /* Check bitlength of mapped object */
   if (entry->bitlength != mapped_bitlength)
   {
      return CO_SDO_ABORT_UNMAPPABLE;
   }

   /* Check that RPDO is mappable */
   if (is_rx && (entry->flags & OD_RPDO) == 0)
   {
      return CO_SDO_ABORT_UNMAPPABLE;
   }

   /* Check that TPDO is mappable */
   if (!is_rx && (entry->flags & OD_TPDO) == 0)
   {
      return CO_SDO_ABORT_UNMAPPABLE;
   }

   /* Save mapping and reference to mapped entry for faster PDO handling */
   pdo->mappings[subindex - 1] = *value;
   pdo->objs[subindex - 1]     = obj;
   pdo->entries[subindex - 1]  = entry;

   return 0;
}

void co_pdo_mapping_init (co_net_t * net)
{
   unsigned int ix;

   for (ix = 0; ix < MAX_RX_PDO; ix++)
   {
      co_pdo_t * pdo = &net->pdo_rx[ix];
      if ((pdo->cobid & CO_COBID_INVALID) == 0)
      {
         co_pdo_mapping_validate (pdo, pdo->number_of_mappings);
      }
   }

   for (ix = 0; ix < MAX_TX_PDO; ix++)
   {
      co_pdo_t * pdo = &net->pdo_tx[ix];
      if ((pdo->cobid & CO_COBID_INVALID) == 0)
      {
         co_pdo_mapping_validate (pdo, pdo->number_of_mappings);
      }
   }
}

uint32_t co_od1007_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   switch (event)
   {
   case OD_EVENT_READ:
      *value = net->sync_window;
      return 0;

   case OD_EVENT_WRITE:
      net->sync_window = *value;
      return 0;

   case OD_EVENT_RESTORE:
      net->sync_window = 0;
      return 0;

   default:
      return CO_SDO_ABORT_GENERAL;
   }
   return 0;
}

static co_pdo_t * co_pdo_find (co_net_t * net, uint16_t index)
{
   bool is_rx = (index & 0x0800) == 0;
   co_pdo_t * pdo;
   size_t size;

   if (is_rx)
   {
      pdo  = net->pdo_rx;
      size = MAX_RX_PDO;
   }
   else
   {
      pdo  = net->pdo_tx;
      size = MAX_TX_PDO;
   }

   for (size_t ix = 0; ix < size; ix++)
   {
      if (pdo[ix].number == (index & 0x01FF))
         return &pdo[ix];
   }

   return NULL;
}

uint32_t co_od1400_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   co_pdo_t * pdo = co_pdo_find (net, obj->index);

   CC_ASSERT (pdo != NULL);
   switch (event)
   {
   case OD_EVENT_READ:
      return co_pdo_comm_get (net, pdo, subindex, value);

   case OD_EVENT_WRITE:
      return co_pdo_comm_set (net, pdo, subindex, value, true);

   case OD_EVENT_RESTORE:
      pdo->cobid =
         CO_COBID_INVALID |
         ((pdo->number < 4) ? (net->node + 0x200 + pdo->number * 0x100) : 0);
      pdo->transmission_type = 0xFF;
      pdo->inhibit_time      = 0;
      pdo->event_timer       = 0;
      pdo->sync_start        = 0;
      return 0;

   default:
      return CO_SDO_ABORT_GENERAL;
   }
}

uint32_t co_od1600_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   co_pdo_t * pdo = co_pdo_find (net, obj->index);

   CC_ASSERT (pdo != NULL);
   switch (event)
   {
   case OD_EVENT_READ:
      return co_pdo_map_get (net, pdo, subindex, value);

   case OD_EVENT_WRITE:
      return co_pdo_map_set (net, pdo, subindex, value, true);

   case OD_EVENT_RESTORE:
      pdo->number_of_mappings = MAX_PDO_ENTRIES;
      memset (pdo->mappings, 0, sizeof (pdo->mappings));
      return 0;

   default:
      return CO_SDO_ABORT_GENERAL;
   }
}

uint32_t co_od1800_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   co_pdo_t * pdo = co_pdo_find (net, obj->index);

   CC_ASSERT (pdo != NULL);
   switch (event)
   {
   case OD_EVENT_READ:
      return co_pdo_comm_get (net, pdo, subindex, value);

   case OD_EVENT_WRITE:
      return co_pdo_comm_set (net, pdo, subindex, value, false);

   case OD_EVENT_RESTORE:
      pdo->cobid =
         CO_COBID_INVALID |
         ((pdo->number < 4) ? (net->node + 0x180 + pdo->number * 0x100) : 0);
      pdo->transmission_type = 0xFF;
      pdo->inhibit_time      = 0;
      pdo->event_timer       = 0;
      pdo->sync_start        = 0;
      return 0;

   default:
      return CO_SDO_ABORT_GENERAL;
   }
}

uint32_t co_od1A00_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   co_pdo_t * pdo = co_pdo_find (net, obj->index);

   CC_ASSERT (pdo != NULL);
   switch (event)
   {
   case OD_EVENT_READ:
      return co_pdo_map_get (net, pdo, subindex, value);

   case OD_EVENT_WRITE:
      return co_pdo_map_set (net, pdo, subindex, value, false);

   case OD_EVENT_RESTORE:
      pdo->number_of_mappings = MAX_PDO_ENTRIES;
      memset (pdo->mappings, 0, sizeof (pdo->mappings));
      return 0;

   default:
      return CO_SDO_ABORT_GENERAL;
   }
}

static void co_pdo_transmit (co_net_t * net, co_pdo_t * pdo)
{
   size_t dlc;
   uint32_t now = os_get_current_time_us();

   if (IS_EVENT (pdo->transmission_type) && pdo->inhibit_time > 0)
   {
      /* Check that inhibit time has expired */
      if (!co_is_expired (now, pdo->timestamp, 100 * pdo->inhibit_time))
         return;
   }

   /* Transmit PDO */
   co_pdo_pack (net, pdo);
   dlc = CO_BYTELENGTH (pdo->bitlength);
   os_channel_send (net->channel, pdo->cobid & CO_EXTID_MASK, &pdo->frame, dlc);
   pdo->timestamp = now;
   pdo->queued    = false;
}

int co_pdo_timer (co_net_t * net, uint32_t now)
{
   unsigned int ix;

   if (net->state != STATE_OP)
      return -1;

   /* Check for TPDOs with event timer */
   for (ix = 0; ix < MAX_TX_PDO; ix++)
   {
      co_pdo_t * pdo = &net->pdo_tx[ix];

      if (pdo->cobid & CO_COBID_INVALID)
         continue;

      if (!IS_EVENT (pdo->transmission_type) || pdo->event_timer == 0)
         continue;

      if (co_is_expired (now, pdo->timestamp, 1000 * pdo->event_timer))
      {
         /* Event timer has expired, transmit PDO */
         co_pdo_transmit (net, pdo);
      }
   }

   return 0;
}

void co_pdo_trigger (co_net_t * net)
{
   unsigned int ix;

   if (net->state != STATE_OP)
      return;

   /* Transmit event-driven TPDOs, queue acyclic TPDOs */
   for (ix = 0; ix < MAX_TX_PDO; ix++)
   {
      co_pdo_t * pdo = &net->pdo_tx[ix];

      if (pdo->cobid & CO_COBID_INVALID)
         continue;

      if (IS_EVENT (pdo->transmission_type))
      {
         co_pdo_transmit (net, pdo);
      }
      else if (IS_ACYCLIC (pdo->transmission_type))
      {
         pdo->queued = true;
      }
   }
}

void co_pdo_trigger_with_obj (co_net_t * net, uint16_t index, uint8_t subindex)
{
   unsigned int ix;
   uint8_t n;
   const co_obj_t * obj;
   const co_entry_t * entry;

   if (net->state != STATE_OP)
      return;

   /* Find mapped object */
   obj = co_obj_find (net, index);
   if (obj == NULL)
      return;

   /* Find mapped entry */
   entry = co_entry_find (net, obj, subindex);
   if (entry == NULL)
      return;

   /* Check that object is mappable */
   if ((entry->flags & OD_TPDO) == 0)
      return;

   /* Transmit event-driven TPDOs, queue acyclic TPDOs */
   for (ix = 0; ix < MAX_TX_PDO; ix++)
   {
      co_pdo_t * pdo = &net->pdo_tx[ix];
      if (pdo->cobid & CO_COBID_INVALID)
         continue;
         
      for (n = 0; n < pdo->number_of_mappings; n++)
      {
         if (pdo->entries[n] == entry)
         {
            if (IS_EVENT (pdo->transmission_type))
            {
               co_pdo_transmit (net, pdo);
            }
            else if (IS_ACYCLIC (pdo->transmission_type))
            {
               pdo->queued = true;
            }
            break;
         }
      }
   }
}

int co_pdo_sync (co_net_t * net, uint8_t * msg, size_t dlc)
{
   unsigned int ix;

   if (net->state != STATE_OP)
      return -1;

   net->sync_timestamp = os_get_current_time_us();

   /* Transmit TPDOs */
   for (ix = 0; ix < MAX_TX_PDO; ix++)
   {
      co_pdo_t * pdo = &net->pdo_tx[ix];

      if (pdo->cobid & CO_COBID_INVALID)
         continue;

      if (pdo->queued)
      {
         /* Queued by event */
         co_pdo_transmit (net, pdo);
      }
      else if (IS_CYCLIC (pdo->transmission_type))
      {
         /* Check SYNC start value */
         if (pdo->sync_wait)
         {
            if (net->sync.overflow == 0)
            {
               /* SYNC counter not enabled, ignore SYNC start */
               pdo->sync_wait = false;
            }
            else if (IS_CYCLIC (pdo->sync_start))
            {
               uint8_t counter = co_fetch_uint8 (msg);
               if (counter == pdo->sync_start)
               {
                  /* SYNC counter wait, this is the first SYNC message */
                  pdo->sync_wait = false;
               }
            }
         }

         if (!pdo->sync_wait)
         {
            pdo->sync_counter += 1;
            if (pdo->sync_counter == pdo->transmission_type)
            {
               co_pdo_transmit (net, pdo);
               pdo->sync_counter = 0;
            }
         }
      }
      else if (pdo->transmission_type == CO_PDO_TT_RTR_SYNC)
      {
         /* Sample now, transmit at RTR */
         co_pdo_pack (net, pdo);
      }
   }

   /* Deliver queued RPDOs */
   for (ix = 0; ix < MAX_RX_PDO; ix++)
   {
      co_pdo_t * pdo = &net->pdo_rx[ix];
      if (pdo->queued)
      {
         co_pdo_unpack (net, pdo);
         pdo->queued = false;
      }
   }

   /* Call user callback */
   if (net->cb_sync)
   {
      net->cb_sync (net);
   }

   return 0;
}

void co_pdo_rx (co_net_t * net, uint32_t id, void * msg, size_t dlc)
{
   unsigned int ix;
   uint32_t now;

   /* Check state */
   if (net->state != STATE_OP)
      return;

   if (id & CO_RTR_MASK)
   {
      id &= CO_EXTID_MASK;
      for (ix = 0; ix < MAX_TX_PDO; ix++)
      {
         co_pdo_t * pdo = &net->pdo_tx[ix];

         if (pdo->cobid == id)
         {
            if (IS_EVENT (pdo->transmission_type) || pdo->transmission_type == CO_PDO_TT_RTR_EVENT)
            {
               /* Sample and transmit immediately */
               co_pdo_transmit (net, pdo);
            }
            else if (
               pdo->transmission_type <= CO_PDO_TT_CYCLIC_MAX ||
               pdo->transmission_type == CO_PDO_TT_RTR_SYNC)
            {
               /* Transmit value sampled at previous SYNC */
               dlc = CO_BYTELENGTH (pdo->bitlength);
               os_channel_send (net->channel, pdo->cobid, &pdo->frame, dlc);
               pdo->timestamp = os_get_current_time_us();
               pdo->queued    = false;
            }
         }
      }
   }
   else
   {
      for (ix = 0; ix < MAX_RX_PDO; ix++)
      {
         co_pdo_t * pdo = &net->pdo_rx[ix];

         if (pdo->cobid == id)
         {
            if (CO_BYTELENGTH (pdo->bitlength) > dlc)
            {
               /* PDO received is too short. Sending EMCY when it's too long is
                * optional, so don't do that (data must still be consumed). */
               co_emcy_tx (net, 0x8210, 0, NULL);
            }
            else
            {
               if (pdo->transmission_type <= CO_PDO_TT_CYCLIC_MAX && net->sync_window > 0)
               {
                  /* Check that sync window has not expired */
                  now = os_get_current_time_us();
                  if (co_is_expired (now, net->sync_timestamp, net->sync_window))
                     continue;
               }

               /* Buffer frame */
               memcpy (&pdo->frame, msg, dlc);
               pdo->timestamp = os_get_current_time_us();

               if (IS_EVENT (pdo->transmission_type))
               {
                  /* Deliver event-driven RPDOs asynchronously */
                  co_pdo_unpack (net, pdo);
               }
               else
               {
                  /* Deliver synchronously */
                  pdo->queued = true;
               }
            }
         }
      }
   }
}

void co_pdo_job (co_net_t * net, co_job_t * job)
{
   switch (job->type)
   {
   case CO_JOB_PDO_EVENT:
      co_pdo_trigger (net);
      break;
   case CO_JOB_PDO_OBJ_EVENT:
      co_pdo_trigger_with_obj (net, job->pdo.index, job->pdo.subindex);
      break;
   default:
      CC_ASSERT (0);
   }

   job->result = 0;
   if (job->callback)
      job->callback (job);
}

int co_pdo_init (co_net_t * net)
{
   const co_obj_t * obj = net->od;
   int pdo_rx           = 0;
   int pdo_tx           = 0;
   int ix;

   /* Disable RPDOs */
   for (ix = 0; ix < MAX_RX_PDO; ix++)
   {
      net->pdo_rx[ix].cobid = CO_COBID_INVALID;
   }

   /* Disable TPDOs */
   for (ix = 0; ix < MAX_TX_PDO; ix++)
   {
      net->pdo_tx[ix].cobid = CO_COBID_INVALID;
   }

   /* Walk dictionary to find available PDO numbers  */
   while (obj->index != 0)
   {
      if (obj->index >= 0x1400 && obj->index < 0x1600)
      {
         /* Out of RPDOs? */
         if (pdo_rx == MAX_RX_PDO)
            return -1;

         net->pdo_rx[pdo_rx++].number = obj->index - 0x1400;
      }
      else if (obj->index >= 0x1800 && obj->index < 0x1A00)
      {
         /* Out of TPDOs? */
         if (pdo_tx == MAX_TX_PDO)
            return -1;

         net->pdo_tx[pdo_tx++].number = obj->index - 0x1800;
      }

      obj++;
   }

   return 0;
}
