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
#define os_usleep              mock_os_usleep
#define os_thread_create       mock_os_thread_create
#define os_channel_open        mock_os_channel_open
#define os_channel_send        mock_os_channel_send
#define os_channel_receive     mock_os_channel_receive
#define os_channel_set_bitrate mock_os_channel_set_bitrate
#define os_channel_set_filter  mock_os_channel_set_filter
#define os_channel_bus_on      mock_os_channel_bus_on
#define os_channel_bus_off     mock_os_channel_bus_off
#endif

#include "co_main.h"
#include "co_api.h"

#include "co_nmt.h"
#include "co_sdo.h"
#include "co_pdo.h"
#include "co_sync.h"
#include "co_emcy.h"
#include "co_heartbeat.h"
#include "co_node_guard.h"
#include "co_lss.h"
#include "co_bitmap.h"

#include <stdio.h>
#include <stdlib.h>

#define IS_PDO(f) ((f) >= CO_FUNCTION_PDO1_TX && (f) <= CO_FUNCTION_PDO4_RX)

void co_handle_rx (co_net_t * net)
{
   int status;
   uint32_t id;
   uint8_t data[8];
   size_t dlc;

   do
   {
      status = os_channel_receive (net->channel, &id, data, &dlc);
      if (status == 0)
      {
         uint16_t function = id & CO_FUNCTION_MASK;
         uint8_t node      = CO_NODE_GET (id);

         /* Process messages */
         if (function == CO_FUNCTION_NMT)
         {
            co_nmt_rx (net, id, data, dlc);
         }
         else if (IS_PDO (function))
         {
            co_pdo_rx (net, id, data, dlc);
         }
         else if (function == CO_FUNCTION_SYNC && node == 0)
         {
            co_pdo_sync (net, data, dlc);
         }
         else if (function == CO_FUNCTION_EMCY)
         {
            co_emcy_rx (net, id, data, dlc);
         }
         else if (function == CO_FUNCTION_SDO_TX)
         {
            co_sdo_tx (net, node, data, dlc);
         }
         else if (function == CO_FUNCTION_SDO_RX)
         {
            co_sdo_rx (net, node, data, dlc);
         }
         else if (function == CO_FUNCTION_NMT_ERR)
         {
            co_heartbeat_rx (net, node, data, dlc);
            co_node_guard_rx (net, id, data, dlc);
         }
         else if (function == CO_FUNCTION_LSS)
         {
            co_lss_rx (net, id, data, dlc);
         }
      }
   } while (status == 0);
}

void co_handle_periodic (co_net_t * net)
{
   uint32_t now = os_get_current_time_us();

   co_sdo_server_timer (net, now);
   co_sdo_client_timer (net, now);
   co_pdo_timer (net, now);
   co_sync_timer (net, now);
   co_heartbeat_timer (net, now);
   co_node_guard_timer (net, now);
}

void co_main (void * arg)
{
   co_net_t * net = arg;
   co_job_t * job;
   bool running = true;

   /* Main loop */
   while (running)
   {
      os_mbox_fetch (net->mbox, (void **)&job, OS_WAIT_FOREVER);

      switch (job->type)
      {
      case CO_JOB_PERIODIC:
         co_handle_periodic (net);
         co_emcy_handle_can_state (net);
         break;
      case CO_JOB_RX:
         co_handle_rx (net);
         break;
      case CO_JOB_PDO_EVENT:
      case CO_JOB_PDO_OBJ_EVENT:
         co_pdo_job (net, job);
         break;
      case CO_JOB_SDO_READ:
      case CO_JOB_SDO_WRITE:
         co_sdo_issue (net, job);
         break;
      case CO_JOB_EMCY_TX:
      case CO_JOB_ERROR_SET:
      case CO_JOB_ERROR_CLEAR:
      case CO_JOB_ERROR_GET:
         co_emcy_job (net, job);
         break;
      case CO_JOB_EXIT:
         running = false;
         break;
      default:
         CC_ASSERT (0);
         break;
      }
   }
}

static void co_timer (os_timer_t * timer, void * arg)
{
   co_net_t * net = arg;
   int tmo;

   tmo = os_mbox_post (net->mbox, &net->job_periodic, 0);
   if (tmo)
   {
      net->mbox_overrun++;
   }
}

static void co_can_callback (co_net_t * net)
{
   int tmo;

   tmo = os_mbox_post (net->mbox, &net->job_rx, 0);
   if (tmo)
   {
      net->mbox_overrun++;
   }
}

static void co_job_callback (co_job_t * job)
{
   co_client_t * client = job->client;
   os_sem_signal (client->sem);
}

/* TODO: issue nmt job? */
void co_nmt (co_client_t * client, co_nmt_cmd_t cmd, uint8_t node)
{
   co_net_t * net = client->net;
   uint8_t data[] = {cmd, node};

   if (node == net->node || node == 0)
   {
      co_nmt_rx (net, 0, data, sizeof (data));
   }

   os_channel_send (net->channel, CO_FUNCTION_NMT, data, sizeof (data));
}

/* TODO: issue sync job? */
void co_sync (co_client_t * client)
{
   co_net_t * net = client->net;

   co_pdo_sync (net, NULL, 0);
   os_channel_send (net->channel, CO_FUNCTION_SYNC, NULL, 0);
}

uint8_t co_node_next (co_client_t * client, uint8_t node)
{
   co_net_t * net = client->net;

   if (node > 127)
      return 0;

   return co_bitmap_next (net->nodes, node);
}

uint8_t co_node_id_get (co_net_t * net)
{
   return net->node;
}

void * co_cb_arg_get (co_net_t * net)
{
   return net->cb_arg;
}

int co_pdo_event (co_client_t * client)
{
   co_net_t * net = client->net;
   co_job_t * job = &client->job;

   job->client   = client;
   job->callback = NULL;
   job->type     = CO_JOB_PDO_EVENT;
   job->callback = co_job_callback;

   os_mbox_post (net->mbox, job, OS_WAIT_FOREVER);
   os_sem_wait (client->sem, OS_WAIT_FOREVER);

   return 0;
}

int co_pdo_obj_event (co_client_t * client, uint16_t index, uint8_t subindex)
{
   co_net_t * net = client->net;
   co_job_t * job = &client->job;

   job->client       = client;
   job->callback     = NULL;
   job->type         = CO_JOB_PDO_OBJ_EVENT;
   job->pdo.index    = index;
   job->pdo.subindex = subindex;
   job->callback     = co_job_callback;

   os_mbox_post (net->mbox, job, OS_WAIT_FOREVER);
   os_sem_wait (client->sem, OS_WAIT_FOREVER);

   return 0;
}

int co_sdo_read (
   co_client_t * client,
   uint8_t node,
   uint16_t index,
   uint8_t subindex,
   void * data,
   size_t size)
{
   co_net_t * net = client->net;
   co_job_t * job = &client->job;

   LOG_DEBUG (CO_SDO_LOG, "sdo read %d:%04X:%02X\n", node, index, subindex);

   job->client       = client;
   job->sdo.node     = node;
   job->sdo.index    = index;
   job->sdo.subindex = subindex;
   job->sdo.data     = data;
   job->sdo.remain   = size;
   job->callback     = co_job_callback;
   job->timestamp    = os_get_current_time_us();
   job->type         = CO_JOB_SDO_READ;

   os_mbox_post (net->mbox, job, OS_WAIT_FOREVER);
   os_sem_wait (client->sem, OS_WAIT_FOREVER);

   return job->result;
}

int co_sdo_write (
   co_client_t * client,
   uint8_t node,
   uint16_t index,
   uint8_t subindex,
   const void * data,
   size_t size)
{
   co_net_t * net = client->net;
   co_job_t * job = &client->job;

   LOG_DEBUG (CO_SDO_LOG, "sdo write %d:%04X:%02X\n", node, index, subindex);

   job->client       = client;
   job->sdo.node     = node;
   job->sdo.index    = index;
   job->sdo.subindex = subindex;
   job->sdo.data     = (uint8_t *)data;
   job->sdo.remain   = size;
   job->callback     = co_job_callback;
   job->timestamp    = os_get_current_time_us();
   job->type         = CO_JOB_SDO_WRITE;

   os_mbox_post (net->mbox, job, OS_WAIT_FOREVER);
   os_sem_wait (client->sem, OS_WAIT_FOREVER);

   return job->result;
}

int co_emcy_issue (
   co_client_t * client,
   uint16_t code,
   uint16_t info,
   uint8_t msef[5])
{
   co_net_t * net = client->net;
   co_job_t * job = &client->job;

   job->client    = client;
   job->callback  = co_job_callback;
   job->emcy.code = code;
   job->emcy.info = info;
   job->emcy.msef = msef;
   job->type      = CO_JOB_EMCY_TX;

   os_mbox_post (net->mbox, job, OS_WAIT_FOREVER);
   os_sem_wait (client->sem, OS_WAIT_FOREVER);

   return job->result;
}

int co_error_set (co_client_t * client, uint8_t mask)
{
   co_net_t * net = client->net;
   co_job_t * job = &client->job;

   job->client     = client;
   job->callback   = co_job_callback;
   job->emcy.value = mask;
   job->type       = CO_JOB_ERROR_SET;

   os_mbox_post (net->mbox, job, OS_WAIT_FOREVER);
   os_sem_wait (client->sem, OS_WAIT_FOREVER);

   return job->result;
}

int co_error_clear (co_client_t * client, uint8_t mask)
{
   co_net_t * net = client->net;
   co_job_t * job = &client->job;

   job->client     = client;
   job->callback   = co_job_callback;
   job->emcy.value = mask;
   job->type       = CO_JOB_ERROR_CLEAR;

   os_mbox_post (net->mbox, job, OS_WAIT_FOREVER);
   os_sem_wait (client->sem, OS_WAIT_FOREVER);

   return job->result;
}

int co_error_get (co_client_t * client, uint8_t * error)
{
   co_net_t * net = client->net;
   co_job_t * job = &client->job;

   job->client   = client;
   job->callback = co_job_callback;
   job->type     = CO_JOB_ERROR_GET;

   os_mbox_post (net->mbox, job, OS_WAIT_FOREVER);
   os_sem_wait (client->sem, OS_WAIT_FOREVER);

   *error = job->emcy.value;

   return job->result;
}

co_client_t * co_client_init (co_net_t * net)
{
   co_client_t * client;

   client = calloc (1, sizeof (*client));
   if (client == NULL)
      return NULL;

   client->sem = os_sem_create (0);
   client->net = net;

   return client;
}

co_net_t * co_init (const char * canif, const co_cfg_t * cfg)
{
   co_net_t * net;
   os_timer_t * tmr;

   net = calloc (1, sizeof (*net));
   if (net == NULL)
      goto error1;

   net->bitrate  = cfg->bitrate;
   net->node     = cfg->node;
   net->od       = cfg->od;
   net->defaults = cfg->defaults;

   net->cb_arg    = cfg->cb_arg;
   net->cb_reset  = cfg->cb_reset;
   net->cb_nmt    = cfg->cb_nmt;
   net->cb_sync   = cfg->cb_sync;
   net->cb_emcy   = cfg->cb_emcy;
   net->cb_notify = cfg->cb_notify;

   net->restart_ms = cfg->restart_ms;

   net->open  = cfg->open;
   net->read  = cfg->read;
   net->write = cfg->write;
   net->close = cfg->close;

   net->job_periodic = CO_JOB_PERIODIC;
   net->job_rx       = CO_JOB_RX;

   if (co_pdo_init (net) != 0)
      goto error2;

   net->mbox = os_mbox_create (10);
   if (net->mbox == NULL)
      goto error2;

   tmr = os_timer_create (1000, co_timer, net, false);
   if (tmr == NULL)
      goto error3;

   net->channel = os_channel_open (canif, co_can_callback, net);
   if (net->channel == NULL)
      goto error4;

   if (os_thread_create ("co_thread", CO_THREAD_PRIO, CO_THREAD_STACK_SIZE, co_main, net) == NULL)
      goto error4;

   os_timer_start (tmr);
   co_nmt_init (net);

   return net;

error4:
   os_timer_destroy (tmr);
error3:
   os_mbox_destroy (net->mbox);
error2:
   free (net);
error1:
   return NULL;
}
