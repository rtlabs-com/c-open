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
#define co_nmt_rtr mock_co_nmt_rtr
#define os_channel_send mock_os_channel_send
#define os_channel_receive mock_os_channel_receive
#define os_get_current_time_us mock_os_get_current_time_us
#endif

#include "co_node_guard.h"
#include "co_sdo.h"
#include "co_emcy.h"
#include "co_util.h"

uint32_t co_od100C_fn (
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
      *value = net->node_guard.guard_time;
      return 0;
   case OD_EVENT_WRITE:
      net->node_guard.guard_time = *value;
      return 0;
   case OD_EVENT_RESTORE:
      net->node_guard.guard_time = 0;
      return 0;
   default:
      return CO_SDO_ABORT_GENERAL;
   }
}

uint32_t co_od100D_fn (
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
      *value = net->node_guard.life_time_factor;
      return 0;
   case OD_EVENT_WRITE:
      net->node_guard.life_time_factor = *value;
      return 0;
   case OD_EVENT_RESTORE:
      net->node_guard.life_time_factor = 0;
      return 0;
   default:
      return CO_SDO_ABORT_GENERAL;
   }
}


int co_node_guard_rx (co_net_t * net, uint32_t id, void * msg, size_t dlc)
{
   uint8_t _msg[1];
   uint8_t state;

   if (dlc != 1)
      return -1;

   if (id != (CO_RTR_MASK | CO_FUNCTION_NMT_ERR | net->node))
   {
#if CO_CONF_MNGR > 0
      uint8_t * nmt_state = (uint8_t *) msg;
      uint8_t _node = CO_NODE_GET(id);
      /* Store a time stamp when the NMT state was received */
      if ((dlc == 1) && (_node > 0) && (_node <= CO_CONF_MNGR))
      {
         net->nmt_node_ts_resp[_node - 1] = os_get_current_time_us();
         net->nmt_request[_node - 1] = 0x7F & nmt_state[0];
      }
#else
       return -1;
#endif
   }
   else
   {
      net->node_guard.is_alive = true;
      net->node_guard.timestamp = os_get_current_time_us();

      /* Heartbeat producer (heartbeat is prioritised over node guarding)*/
      if (net->hb_time == 0)
      {
         switch(net->state)
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

         co_put_uint8 (_msg, net->node_guard.toggle | state);
         os_channel_send (net->channel, 0x700 + net->node, _msg, sizeof(_msg));
         net->node_guard.toggle = ~net->node_guard.toggle & 0x80;
      }
   }
   return 0;
}

int co_node_guard_timer (co_net_t * net, uint32_t now)
{
   uint32_t guard_factor =
         (net->node_guard.guard_time * net->node_guard.life_time_factor);

   if (net->state == STATE_INIT)
      return -1;

   /* Node guarding */
   if (guard_factor != 0 && net->node_guard.is_alive)
   {
      /* Check node guarding has not expired */
      if (co_is_expired (now, net->node_guard.timestamp, 1000 * guard_factor))
      {
         /* Expired */
         net->node_guard.is_alive = false;
         LOG_ERROR (CO_NODE_GUARD_LOG, "node guarding expired\n");

         net->emcy.node_guard_error = true;
         co_emcy_error_register_set (net, CO_ERR_COMMUNICATION);
         co_emcy_tx (net, 0x8130, 0, NULL);
      }
   }
#if CO_CONF_MNGR > 0
   /* Evaluate all guarded nodes */
   {
      uint8_t i;

      for ( i = 0; i < CO_CONF_MNGR; i++)
      {
         if (net->nmt_request_node_guard[i] == 1)
         {
            uint32_t guard_time = ((net->nmt_slave_assignment[i] >> 16) & 0xFFFF);
            guard_factor =
                  (guard_time * ((net->nmt_slave_assignment[i] >> 8) & 0xFF));

            if ((guard_factor != 0))
            {
               if (now - net->nmt_node_ts_resp[i] > 1000 * guard_factor)
               {
                  /* Expired */
                  LOG_ERROR (CO_NODE_GUARD_LOG, "remote node guarding expired, node: %d\n", i + 1);
                  net->nmt_request[i] = 1; /* CANopen device missing */
                  co_emcy_tx (net, 0x8130, 0, NULL);

                  /* Stall the next emcy call by the guard factor * guard time */
                  net->nmt_node_ts_resp[i] = now;
               }
               if (now - net->nmt_node_ts_req[i] > 1000 * guard_time)
               {
                  net->nmt_node_ts_req[i] = now;
                  co_nmt_rtr (net, i + 1);
               }
            }
         }
      }
   }
#endif
   return 0;
}
