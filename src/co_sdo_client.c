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
#define os_channel_send    mock_os_channel_send
#define os_channel_receive mock_os_channel_receive
#define co_obj_find        mock_co_obj_find
#define co_entry_find      mock_co_entry_find
#endif

#include "co_sdo.h"
#include "co_od.h"
#include "co_util.h"

#include <inttypes.h>

static void co_sdo_done (co_net_t * net)
{
   co_job_t * job = net->job_client;

   net->job_client = NULL;

   if (job->callback)
      job->callback (job);
}

static int co_sdo_tx_upload_init_rsp (
   co_net_t * net,
   uint8_t node,
   uint8_t type,
   uint8_t * data)
{
   co_job_t * job = net->job_client;

   /* Complete if e = 1 */
   if (type & CO_SDO_E)
   {
      size_t size = (type & CO_SDO_S) ? 4 - CO_SDO_N (type) : 4;

      size = MIN (job->sdo.remain, size);
      memcpy (job->sdo.data, &data[4], size);

      job->sdo.data += size;
      job->sdo.remain -= size;
      job->sdo.total += size;

      job->result = job->sdo.total; /* actual size */
      co_sdo_done (net);
      return 1;
   }
   else
   {
      uint8_t msg[8] = {0};
      size_t size    = co_fetch_uint32 (&data[4]);
      size           = MIN (job->sdo.remain, size);

      job->sdo.remain = size;
      job->sdo.toggle = 0;
      job->sdo.total  = 0;

      msg[0] = CO_SDO_CCS_UPLOAD_SEG_REQ;

      os_channel_send (net->channel, 0x600 + node, msg, sizeof (msg));
   }

   return 0;
}

static int co_sdo_tx_upload_seg_rsp (
   co_net_t * net,
   uint8_t node,
   uint8_t type,
   uint8_t * data)
{
   co_job_t * job = net->job_client;
   int error;

   error = co_sdo_toggle_update (job, type);
   if (error < 0)
   {
      co_sdo_abort (
         net,
         0x600 + net->node,
         job->sdo.index,
         job->sdo.subindex,
         CO_SDO_ABORT_TOGGLE);
      job->result = CO_STATUS_ERROR;
      co_sdo_done (net);
      return error;
   }

   size_t size = 7 - CO_SDO_N_SEG (type);
   size        = MIN (job->sdo.remain, size);
   memcpy (job->sdo.data, &data[1], size);

   job->sdo.data += size;
   job->sdo.remain -= size;
   job->sdo.total += size;

   /* Complete if c = 1 */
   if (type & CO_SDO_C)
   {
      job->result = job->sdo.total;
      co_sdo_done (net);
      return 1;
   }
   else
   {
      uint8_t msg[8] = {0};

      msg[0] = CO_SDO_CCS_UPLOAD_SEG_REQ;
      if (job->sdo.toggle)
         msg[0] |= CO_SDO_TOGGLE;

      os_channel_send (net->channel, 0x600 + node, msg, sizeof (msg));
   }

   return 0;
}

static int co_sdo_tx_download_init_rsp (
   co_net_t * net,
   uint8_t node,
   uint8_t type,
   uint8_t * data)
{
   co_job_t * job = net->job_client;

   if (job->sdo.remain == 0)
   {
      /* Complete */
      job->result = job->sdo.total;
      co_sdo_done (net);
      return 1;
   }
   else
   {
      uint8_t msg[8] = {0};

      size_t size = MIN (job->sdo.remain, 7);
      memcpy (&msg[1], job->sdo.data, size);

      msg[0] = CO_SDO_CCS_DOWNLOAD_SEG_REQ | ((7 - (size & 0x07)) << 1);
      if (size < 7)
         msg[0] |= CO_SDO_C;

      job->sdo.toggle = 0;

      job->sdo.data += size;
      job->sdo.remain -= size;
      job->sdo.total += size;

      os_channel_send (net->channel, 0x600 + node, msg, sizeof (msg));
   }

   return 0;
}

static int co_sdo_tx_download_seg_rsp (
   co_net_t * net,
   uint8_t node,
   uint8_t type,
   uint8_t * data)
{
   co_job_t * job = net->job_client;
   int error;

   error = co_sdo_toggle_update (job, type);
   if (error < 0)
   {
      co_sdo_abort (
         net,
         0x600 + net->node,
         job->sdo.index,
         job->sdo.subindex,
         CO_SDO_ABORT_TOGGLE);
      job->result = CO_STATUS_ERROR;
      co_sdo_done (net);
      return error;
   }

   if (job->sdo.remain == 0)
   {
      /* Complete */
      job->result = job->sdo.total;
      co_sdo_done (net);
      return 1;
   }
   else
   {
      uint8_t msg[8] = {0};

      size_t size = MIN (job->sdo.remain, 7);
      memcpy (&msg[1], job->sdo.data, size);

      msg[0] = CO_SDO_CCS_DOWNLOAD_SEG_REQ | ((7 - (size & 0x07)) << 1);
      if (job->sdo.toggle)
         msg[0] |= CO_SDO_TOGGLE;
      if (size < 7)
         msg[0] |= CO_SDO_C;

      job->sdo.data += size;
      job->sdo.remain -= size;
      job->sdo.total += size;

      os_channel_send (net->channel, 0x600 + node, msg, sizeof (msg));
   }

   return 0;
}

int co_sdo_tx (co_net_t * net, uint8_t node, void * msg, size_t dlc)
{
   uint8_t * data = (uint8_t *)msg;
   uint8_t type   = data[0];
   uint8_t scs    = CO_SDO_xCS (type);
   co_job_t * job = net->job_client;

   /* Check for ongoing job */
   if (job == NULL)
      return -1;

   /* Check for correct response */
   if (job->sdo.node != node)
      return -1;

   /* Check DLC - must be complete frame */
   if (dlc != 8)
   {
      co_sdo_abort (net, 0x600 + net->node, 0, 0, CO_SDO_ABORT_GENERAL);
      job->result = CO_STATUS_ERROR;
      co_sdo_done (net);
      return -1;
   }

   /* Check response type */
   switch (scs)
   {
   case CO_SDO_SCS_UPLOAD_INIT_RSP:
      return co_sdo_tx_upload_init_rsp (net, node, type, data);

   case CO_SDO_SCS_UPLOAD_SEG_RSP:
      return co_sdo_tx_upload_seg_rsp (net, node, type, data);

   case CO_SDO_SCS_DOWNLOAD_INIT_RSP:
      return co_sdo_tx_download_init_rsp (net, node, type, data);

   case CO_SDO_SCS_DOWNLOAD_SEG_RSP:
      return co_sdo_tx_download_seg_rsp (net, node, type, data);

   case CO_SDO_xCS_ABORT:
   {
      uint32_t error = co_fetch_uint32 (&data[4]);
      (void)error;
      LOG_WARNING (CO_SDO_LOG, "sdo abort (%08" PRIx32 ")\n", error);
      job->result = CO_STATUS_ERROR;
      co_sdo_done (net);
      return 1;
   }

   default:
      co_sdo_abort (net, 0x600 + net->node, 0, 0, CO_SDO_ABORT_UNKNOWN);
      LOG_ERROR (CO_SDO_LOG, "sdo unknown command (%X)\n", scs);
      job->result = CO_STATUS_ERROR;
      co_sdo_done (net);
      return 1;
   }
}

void co_sdo_issue (co_net_t * net, co_job_t * job)
{
   uint8_t msg[8] = {0};

   net->job_client = job;
   job->sdo.total  = 0;

   if (job->type == CO_JOB_SDO_READ)
   {
      msg[0] = CO_SDO_CCS_UPLOAD_INIT_REQ;
   }
   else
   {
      msg[0] = CO_SDO_CCS_DOWNLOAD_INIT_REQ;
      if (job->sdo.remain <= 4)
      {
         int n = 4 - job->sdo.remain;
         msg[0] |= (n << 2) | CO_SDO_E | CO_SDO_S;
         memcpy (&msg[4], job->sdo.data, job->sdo.remain);
         job->sdo.total  = job->sdo.remain;
         job->sdo.remain = 0;
      }
      else
      {
         msg[0] |= CO_SDO_S;
         co_put_uint32 (&msg[4], job->sdo.remain);
      }
   }

   co_put_uint16 (&msg[1], job->sdo.index);
   co_put_uint8 (&msg[3], job->sdo.subindex);

   os_channel_send (net->channel, 0x600 + job->sdo.node, msg, sizeof (msg));
}

int co_sdo_client_timer (co_net_t * net, uint32_t now)
{
   co_job_t * job = net->job_client;

   if (job == NULL)
      return 0;

   if (job->type == CO_JOB_SDO_READ || job->type == CO_JOB_SDO_WRITE)
   {
      if (co_is_expired (now, job->timestamp, 1000 * SDO_TIMEOUT))
      {
         co_sdo_abort (
            net,
            0x580 + net->node,
            job->sdo.index,
            job->sdo.subindex,
            CO_SDO_ABORT_TIMEOUT);

         job->result = CO_STATUS_ERROR;
         co_sdo_done (net);
      }
   }

   return 0;
}
