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
#define co_emcy_tx             mock_co_emcy_tx
#endif

#include "co_heartbeat.h"
#include "co_sdo.h"
#include "co_emcy.h"
#include "co_util.h"
#include "co_bitmap.h"

#include <string.h>

uint32_t co_od1017_fn (
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
      *value = net->hb_time;
      return 0;
   case OD_EVENT_WRITE:
      net->hb_time = *value;
      return 0;
   case OD_EVENT_RESTORE:
      net->hb_time = 0;
      return 0;
   default:
      return CO_SDO_ABORT_GENERAL;
   }
}

uint32_t co_od1016_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   co_heartbeat_t * heartbeat = &net->heartbeat[subindex - 1];

   if (event != OD_EVENT_RESTORE)
   {
      if (subindex == 0 || subindex > MAX_HEARTBEATS)
         return CO_SDO_ABORT_BAD_SUBINDEX;
   }

   if (event == OD_EVENT_READ)
   {
      *value = heartbeat->node << 16 | heartbeat->time;
   }
   else if (event == OD_EVENT_WRITE)
   {
      uint8_t node  = (*value >> 16) & 0xFF;
      uint16_t time = *value & 0xFFFF;
      int ix;

      if (node != 0)
      {
         for (ix = 0; ix < MAX_HEARTBEATS; ix++)
         {
            if (ix == subindex - 1)
               continue; /* Ignore this slot */

            if (net->heartbeat[ix].node == node)
               return CO_SDO_ABORT_PARAM_INCOMPATIBLE;
         }
      }

      heartbeat->node = node;
      heartbeat->time = time;
   }
   else if (event == OD_EVENT_RESTORE)
   {
      memset (&net->heartbeat, 0, sizeof (net->heartbeat));
   }

   return 0;
}

int co_heartbeat_rx (co_net_t * net, uint8_t node, void * msg, size_t dlc)
{
   int ix;

   co_bitmap_set (net->nodes, node);

   for (ix = 0; ix < MAX_HEARTBEATS; ix++)
   {
      if (net->heartbeat[ix].node == node)
      {
         co_heartbeat_t * heartbeat = &net->heartbeat[ix];

         heartbeat->timestamp = os_get_current_time_us();
         heartbeat->is_alive  = true;
         LOG_DEBUG (CO_HEARTBEAT_LOG, "node %d got heartbeat\n", heartbeat->node);
      }
   }

   return 0;
}

int co_heartbeat_timer (co_net_t * net, uint32_t now)
{
   unsigned int ix;
   bool heartbeat_error = false;

   /* TODO: send on activation */
   if (net->state == STATE_INIT)
      return -1;

   /* Heartbeat producer */
   if (net->hb_time != 0)
   {
      if (co_is_expired (now, net->hb_timestamp, 1000 * net->hb_time))
      {
         uint8_t msg[1];
         uint8_t state;

         net->hb_timestamp = now;

         switch (net->state)
         {
         case STATE_STOP:
            state = 4;
            break;
         case STATE_OP:
            state = 5;
            break;
         case STATE_PREOP:
            state = 127;
            break;
         default:
            state = 0;
            break;
         }

         co_put_uint8 (msg, state);
         os_channel_send (net->channel, 0x700 + net->node, msg, sizeof (msg));
      }
   }

   /* Heartbeat consumer */
   for (ix = 0; ix < MAX_HEARTBEATS; ix++)
   {
      co_heartbeat_t * heartbeat = &net->heartbeat[ix];

      /* Check active slot */
      if (heartbeat->node == 0 || heartbeat->node > 127)
         continue;

      /* Check that heartbeat has not already expired */
      if (!heartbeat->is_alive)
         continue;

      /* Check heartbeat has not expired */
      if (co_is_expired (now, heartbeat->timestamp, 1000 * heartbeat->time))
      {
         /* Expired */
         heartbeat->is_alive = false;
         co_bitmap_clear (net->nodes, heartbeat->node);
         LOG_ERROR (
            CO_HEARTBEAT_LOG,
            "node %d heartbeat expired\n",
            heartbeat->node);

         heartbeat_error = true;
         co_emcy_error_register_set (net, CO_ERR_COMMUNICATION);
         co_emcy_tx (net, 0x8130, 0, NULL);
      }
   }

   /* Update heartbeat state */
   net->emcy.heartbeat_error = heartbeat_error;

   return 0;
}
