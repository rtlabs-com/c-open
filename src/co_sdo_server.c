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
#define os_get_current_time_us mock_os_get_current_time_us
#define os_channel_send    mock_os_channel_send
#define os_channel_receive mock_os_channel_receive
#define co_obj_find        mock_co_obj_find
#define co_entry_find      mock_co_entry_find
#endif

#include "co_sdo.h"
#include "co_od.h"
#include "co_util.h"

#include <inttypes.h>

#define CO_SDO_xCS(v) ((v)&0xE0)
#define CO_SDO_N(v)   (((v) >> 2) & 0x03)
#define CO_SDO_E      BIT (1)
#define CO_SDO_S      BIT (0)

#define CO_SDO_CCS_DOWNLOAD_SEG_REQ  (0 << 5)
#define CO_SDO_CCS_DOWNLOAD_INIT_REQ (1 << 5)
#define CO_SDO_CCS_UPLOAD_INIT_REQ   (2 << 5)
#define CO_SDO_CCS_UPLOAD_SEG_REQ    (3 << 5)

#define CO_SDO_SCS_UPLOAD_SEG_RSP    (0 << 5)
#define CO_SDO_SCS_DOWNLOAD_SEG_RSP  (1 << 5)
#define CO_SDO_SCS_UPLOAD_INIT_RSP   (2 << 5)
#define CO_SDO_SCS_DOWNLOAD_INIT_RSP (3 << 5)

#define CO_SDO_xCS_ABORT (4 << 5)

#define CO_SDO_TOGGLE   BIT (4)
#define CO_SDO_N_SEG(v) (((v) >> 1) & 0x07)
#define CO_SDO_C        BIT (0)

void co_sdo_abort (
   co_net_t * net,
   uint16_t id,
   uint16_t index,
   uint8_t subindex,
   uint32_t code)
{
   uint8_t msg[8] = {0};
   uint8_t * p    = msg;

   net->job_sdo_server.type = CO_JOB_NONE;

   LOG_WARNING (CO_SDO_LOG, "sdo abort 0x%" PRIx32 "\n", code);

   p = co_put_uint8 (p, CO_SDO_xCS_ABORT);
   p = co_put_uint16 (p, index);
   p = co_put_uint8 (p, subindex);
   co_put_uint32 (p, code);

   os_channel_send (net->channel, id, msg, sizeof (msg));
}

int co_sdo_toggle_update (co_job_t * job, uint8_t type)
{
   int toggle = !!(type & CO_SDO_TOGGLE);
   if (job->sdo.toggle != toggle)
   {
      job->result = CO_SDO_TOGGLE;
      return -1;
   }

   job->sdo.toggle = !job->sdo.toggle;
   return 0;
}

static int co_sdo_get_structure (co_net_t * net, const co_obj_t * obj)
{
   uint8_t msg[8] = {0};
   uint8_t * p    = msg;
   co_dtype_t datatype;

   switch (obj->objtype)
   {
   case OTYPE_VAR:
      datatype = obj->entries[0].datatype;
      break;
   case OTYPE_ARRAY:
      datatype = obj->entries[1].datatype;
      break;
   default:
      datatype = 0;
   }

   p = co_put_uint8 (p, CO_SDO_SCS_UPLOAD_INIT_RSP | CO_SDO_E | CO_SDO_S);
   p = co_put_uint16 (p, obj->index);
   p = co_put_uint8 (p, 0xFF);
   co_put_uint32 (p, (datatype << 8) | obj->objtype);

   os_channel_send (net->channel, 0x580 + net->node, msg, sizeof (msg));
   return 0;
}

static int co_sdo_rx_upload_init_req (
   co_net_t * net,
   uint8_t node,
   uint8_t type,
   uint8_t * data)
{
   co_job_t * job = &net->job_sdo_server;
   const co_obj_t * obj;
   const co_entry_t * entry;
   uint32_t abort;
   uint8_t msg[8] = {0};
   uint8_t * p    = msg;
   uint8_t scs;

   /* Configure upload job */
   job->type         = CO_JOB_SDO_UPLOAD;
   job->sdo.index    = co_fetch_uint16 (&data[1]);
   job->sdo.subindex = data[3];
   job->timestamp    = os_get_current_time_us();

   /* Find requested object */
   obj = co_obj_find (net, job->sdo.index);
   if (obj == NULL)
   {
      co_sdo_abort (
         net,
         0x580 + net->node,
         job->sdo.index,
         job->sdo.subindex,
         CO_SDO_ABORT_BAD_INDEX);
      return -1;
   }

   /* Handle subindex FF */
   if (job->sdo.subindex == 0xFF)
   {
      job->type = CO_JOB_NONE;
      return co_sdo_get_structure (net, obj);
   }

   /* Find requested subindex */
   entry = co_entry_find (net, obj, job->sdo.subindex);
   if (entry == NULL)
   {
      co_sdo_abort (
         net,
         0x580 + net->node,
         job->sdo.index,
         job->sdo.subindex,
         CO_SDO_ABORT_BAD_SUBINDEX);
      return -1;
   }

   /* Check read permission */
   if ((entry->flags & OD_READ) == 0)
   {
      co_sdo_abort (
         net,
         0x580 + net->node,
         job->sdo.index,
         job->sdo.subindex,
         CO_SDO_ABORT_ACCESS_WO);
      return -1;
   }

   job->sdo.remain = CO_BYTELENGTH (entry->bitlength);
   job->sdo.toggle = 0;

   if (job->sdo.remain <= sizeof (job->sdo.value))
   {
      /* Object values up to 64 bits are fetched atomically */
      abort =
         co_od_get_value (net, obj, entry, job->sdo.subindex, &job->sdo.value);
      job->sdo.data = (uint8_t *)&job->sdo.value;
   }
   else
   {
      /* Otherwise a pointer is used to access object */
      abort = co_od_get_ptr (net, obj, entry, job->sdo.subindex, &job->sdo.data);
   }

   if (abort)
   {
      co_sdo_abort (net, 0x580 + net->node, job->sdo.index, job->sdo.subindex, abort);
      return -1;
   }

   if (job->sdo.remain <= 4)
   {
      size_t n = 4 - job->sdo.remain;

      /* Expedited upload */
      scs = CO_SDO_SCS_UPLOAD_INIT_RSP | CO_SDO_E;
      scs |= (n << 2) | CO_SDO_S;

      p = co_put_uint8 (p, scs);
      p = co_put_uint16 (p, job->sdo.index);
      p = co_put_uint8 (p, job->sdo.subindex);
      co_put_uint32 (p, job->sdo.value & UINT32_MAX);

      /* Done */
      job->type = CO_JOB_NONE;
   }
   else
   {
      /* Segmented upload */
      scs = CO_SDO_SCS_UPLOAD_INIT_RSP | CO_SDO_S;

      p = co_put_uint8 (p, scs);
      p = co_put_uint16 (p, job->sdo.index);
      p = co_put_uint8 (p, job->sdo.subindex);
      co_put_uint32 (p, job->sdo.remain);
   }

   os_channel_send (net->channel, 0x580 + net->node, msg, sizeof (msg));
   return 0;
}

static int co_sdo_rx_upload_seg_req (
   co_net_t * net,
   uint8_t node,
   uint8_t type,
   uint8_t * data)
{
   co_job_t * job = &net->job_sdo_server;
   int error;
   uint8_t msg[8] = {0};
   uint8_t * p    = msg;
   uint8_t scs;

   /* Check toggle protocol */
   error = co_sdo_toggle_update (job, type);
   if (error < 0)
   {
      co_sdo_abort (
         net,
         0x580 + net->node,
         job->sdo.index,
         job->sdo.subindex,
         CO_SDO_ABORT_TOGGLE);
      return error;
   }

   if (job->sdo.remain <= 7)
   {
      size_t n = 7 - job->sdo.remain;

      /* Complete segmented upload */
      scs = CO_SDO_SCS_UPLOAD_SEG_RSP | CO_SDO_C;
      scs |= data[0] & CO_SDO_TOGGLE;
      scs |= (n << 1);

      p = co_put_uint8 (p, scs);
      memcpy (p, job->sdo.data, job->sdo.remain);

      /* Done */
      job->type = CO_JOB_NONE;
   }
   else
   {
      size_t size = MIN (job->sdo.remain, 7);

      /* Continue segmented upload */
      scs = CO_SDO_SCS_UPLOAD_SEG_RSP;
      scs |= data[0] & CO_SDO_TOGGLE;

      p = co_put_uint8 (p, scs);
      memcpy (p, job->sdo.data, size);

      job->sdo.data += size;
      job->sdo.remain -= size;
      job->timestamp = os_get_current_time_us();
   }

   os_channel_send (net->channel, 0x580 + net->node, msg, sizeof (msg));
   return 0;
}

static int co_sdo_rx_download_init_req (
   co_net_t * net,
   uint8_t node,
   uint8_t type,
   uint8_t * data)
{
   co_job_t * job = &net->job_sdo_server;
   const co_obj_t * obj;
   const co_entry_t * entry;
   uint32_t abort;
   uint8_t msg[8] = {0};
   uint8_t * p    = msg;

   /* Configure download job */
   job->type         = CO_JOB_SDO_DOWNLOAD;
   job->sdo.index    = co_fetch_uint16 (&data[1]);
   job->sdo.subindex = data[3];
   job->timestamp    = os_get_current_time_us();

   /* Find requested object */
   obj = co_obj_find (net, job->sdo.index);
   if (obj == NULL)
   {
      co_sdo_abort (
         net,
         0x580 + net->node,
         job->sdo.index,
         job->sdo.subindex,
         CO_SDO_ABORT_BAD_INDEX);
      return -1;
   }

   /* Find requested subindex */
   entry = co_entry_find (net, obj, job->sdo.subindex);
   if (entry == NULL)
   {
      co_sdo_abort (
         net,
         0x580 + net->node,
         job->sdo.index,
         job->sdo.subindex,
         CO_SDO_ABORT_BAD_SUBINDEX);
      return -1;
   }

   /* Check write permission */
   if ((entry->flags & OD_WRITE) == 0)
   {
      co_sdo_abort (
         net,
         0x580 + net->node,
         job->sdo.index,
         job->sdo.subindex,
         CO_SDO_ABORT_ACCESS_RO);
      return -1;
   }

   job->sdo.remain = CO_BYTELENGTH (entry->bitlength);
   job->sdo.toggle = 0;

   if (job->sdo.remain <= sizeof (job->sdo.value))
   {
      /* Object values up to 64 bits are cached so that we can set
         them atomically when the transfer is complete */
      job->sdo.data   = (uint8_t *)&job->sdo.value;
      job->sdo.cached = true;
   }
   else
   {
      /* Otherwise a pointer is used to access object */
      abort = co_od_get_ptr (net, obj, entry, job->sdo.subindex, &job->sdo.data);
      if (abort)
      {
         co_sdo_abort (
            net,
            0x580 + net->node,
            job->sdo.index,
            job->sdo.subindex,
            abort);
         return -1;
      }
   }

   /* Check for expedited download */
   if (type & CO_SDO_E)
   {
      size_t size = (type & CO_SDO_S) ? 4 - CO_SDO_N (type) : 4;
      uint32_t value;

      /* Validate size */
      if (size != job->sdo.remain)
      {
         co_sdo_abort (
            net,
            0x580 + net->node,
            job->sdo.index,
            job->sdo.subindex,
            CO_SDO_ABORT_LENGTH);
         return -1;
      }

      /* Fetch value */
      value = co_fetch_uint32 (&data[4]);

      /* Atomically set value */
      abort = co_od_set_value (net, obj, entry, job->sdo.subindex, value);

      /* Done */
      job->type = CO_JOB_NONE;

      if (abort)
      {
         co_sdo_abort (
            net,
            0x580 + net->node,
            job->sdo.index,
            job->sdo.subindex,
            abort);
         return -1;
      }
   }

   /* Dictionary has been written to and is now dirty */
   net->config_dirty = 1;

   /* Send init response */
   p = co_put_uint8 (p, CO_SDO_SCS_DOWNLOAD_INIT_RSP);
   p = co_put_uint16 (p, job->sdo.index);
   co_put_uint8 (p, job->sdo.subindex);

   os_channel_send (net->channel, 0x580 + net->node, msg, sizeof (msg));
   return 0;
}

static int co_sdo_rx_download_seg_req (
   co_net_t * net,
   uint8_t node,
   uint8_t type,
   uint8_t * data)
{
   co_job_t * job = &net->job_sdo_server;
   const co_obj_t * obj;
   const co_entry_t * entry;
   uint32_t abort;
   uint8_t msg[8] = {0};
   uint8_t * p    = msg;
   int error;
   size_t size;
   uint8_t scs;

   /* Check toggle protocol */
   error = co_sdo_toggle_update (job, type);
   if (error < 0)
   {
      co_sdo_abort (
         net,
         0x580 + net->node,
         job->sdo.index,
         job->sdo.subindex,
         CO_SDO_ABORT_TOGGLE);
      return error;
   }

   /* Get data */
   size = 7 - CO_SDO_N_SEG (type);
   size = MIN (size, job->sdo.remain);
   memcpy (job->sdo.data, &data[1], size);

   job->sdo.data += size;
   job->sdo.remain -= size;
   job->timestamp = os_get_current_time_us();

   if (data[0] & CO_SDO_C)
   {
      /* Write complete */
      job->type = CO_JOB_NONE;

      if (job->sdo.cached)
      {
         /* Find requested object */
         obj = co_obj_find (net, job->sdo.index);
         if (obj == NULL)
         {
            co_sdo_abort (
               net,
               0x580 + net->node,
               job->sdo.index,
               job->sdo.subindex,
               CO_SDO_ABORT_BAD_INDEX);
            return -1;
         }

         /* Find requested subindex */
         entry = co_entry_find (net, obj, job->sdo.subindex);
         if (entry == NULL)
         {
            co_sdo_abort (
               net,
               0x580 + net->node,
               job->sdo.index,
               job->sdo.subindex,
               CO_SDO_ABORT_BAD_SUBINDEX);
            return -1;
         }

         /* Atomically set value */
         abort =
            co_od_set_value (net, obj, entry, job->sdo.subindex, job->sdo.value);
         if (abort)
         {
            co_sdo_abort (
               net,
               0x580 + net->node,
               job->sdo.index,
               job->sdo.subindex,
               abort);
            return -1;
         }
      }
   }

   /* Segmented response */
   scs = CO_SDO_SCS_DOWNLOAD_SEG_RSP;
   scs |= data[0] & CO_SDO_TOGGLE;

   co_put_uint8 (p, scs);

   os_channel_send (net->channel, 0x580 + net->node, msg, sizeof (msg));
   return 0;
}

int co_sdo_rx (co_net_t * net, uint8_t node, void * msg, size_t dlc)
{
   uint8_t * data = (uint8_t *)msg;
   uint8_t type   = data[0];
   uint8_t ccs    = CO_SDO_xCS (type);

   /* Check for correct node id */
   if (node != net->node)
      return -1;

   /* Check state */
   if (net->state != STATE_PREOP && net->state != STATE_OP)
   {
      return -1;
   }

   /* Check DLC - must be complete frame */
   if (dlc != 8)
   {
      co_sdo_abort (net, 0x580 + net->node, 0, 0, CO_SDO_ABORT_GENERAL);
      return -1;
   }

   /* Check response type */
   switch (ccs)
   {
   case CO_SDO_CCS_UPLOAD_INIT_REQ:
      return co_sdo_rx_upload_init_req (net, node, type, data);

   case CO_SDO_CCS_UPLOAD_SEG_REQ:
      return co_sdo_rx_upload_seg_req (net, node, type, data);

   case CO_SDO_CCS_DOWNLOAD_INIT_REQ:
      return co_sdo_rx_download_init_req (net, node, type, data);

   case CO_SDO_CCS_DOWNLOAD_SEG_REQ:
      return co_sdo_rx_download_seg_req (net, node, type, data);

   case CO_SDO_xCS_ABORT:
   {
      uint32_t error = co_fetch_uint32 (&data[4]);
      (void)error;
      LOG_WARNING (CO_SDO_LOG, "sdo abort (%08" PRIx32 ")\n", error);
      return 1;
   }

   default:
      co_sdo_abort (net, 0x580 + net->node, 0, 0, CO_SDO_ABORT_UNKNOWN);
      LOG_ERROR (CO_SDO_LOG, "sdo unknown command (%X)\n", ccs);
      return 1;
   }
}

int co_sdo_server_timer (co_net_t * net, uint32_t now)
{
   co_job_t * job = &net->job_sdo_server;

   if (job->type == CO_JOB_SDO_UPLOAD || job->type == CO_JOB_SDO_DOWNLOAD)
   {
      if (co_is_expired (now, job->timestamp, 1000 * SDO_TIMEOUT))
      {
         co_sdo_abort (
            net,
            0x580 + net->node,
            job->sdo.index,
            job->sdo.subindex,
            CO_SDO_ABORT_TIMEOUT);
      }
   }

   return 0;
}
