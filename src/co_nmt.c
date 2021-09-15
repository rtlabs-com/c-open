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
#define os_channel_bus_off     mock_os_channel_bus_off
#define os_channel_bus_on      mock_os_channel_bus_on
#define os_channel_set_bitrate mock_os_channel_set_bitrate
#define os_channel_set_filter  mock_os_channel_set_filter
#define co_od_reset            mock_co_od_reset
#endif

#include "co_nmt.h"
#include "co_od.h"
#include "co_pdo.h"
#include "co_lss.h"

typedef struct co_fsm
{
   co_state_t next;
   co_fsm_event_t (*action) (co_net_t * net, co_fsm_event_t event);
} co_fsm_t;

typedef struct co_fsm_transition
{
   co_state_t state;
   co_fsm_event_t event;
   co_state_t next;
   co_fsm_event_t (*action) (co_net_t * net, co_fsm_event_t event);
} co_fsm_transition_t;

static co_fsm_t fsm[STATE_LAST][EVENT_LAST];

static co_fsm_event_t co_nmt_reset_app (co_net_t * net, co_fsm_event_t event)
{
   /* Reset application and manufacturer-specific area*/
   co_od_reset (net, CO_STORE_MFG, 0x2000, 0x5FFF);
   co_od_reset (net, CO_STORE_APP, 0x6000, 0x9FFF);

   /* Copy persistent node-ID to pending node-ID */
   net->lss.node = co_lss_get_persistent_node_id (net);

   return EVENT_INITDONE;
}

static co_fsm_event_t co_nmt_reset_comm (co_net_t * net, co_fsm_event_t event)
{
   /* Reset communication */
   co_od_reset (net, CO_STORE_COMM, 0x1000, 0x1FFF);
   os_channel_bus_off (net->channel);
   os_channel_set_bitrate (net->channel, net->bitrate);
   os_channel_set_filter (net->channel, NULL, 0);
   os_channel_bus_on (net->channel);

   if (net->lss.node != 0xFF)
   {
      /* Copy pending node-ID to active node-ID */
      net->node = net->lss.node;
      return EVENT_INITDONE;
   }

   /* Remain in INIT_COMM state if pending node-ID invalid */
   return EVENT_NONE;
}

static co_fsm_event_t co_nmt_bootup (co_net_t * net, co_fsm_event_t event)
{
   uint8_t msg[] = {0};
   os_channel_send (
      net->channel,
      CO_FUNCTION_NMT_ERR + net->node,
      msg,
      sizeof (msg));
   return EVENT_NONE;
}

static co_fsm_event_t co_nmt_start (co_net_t * net, co_fsm_event_t event)
{
   co_pdo_mapping_init (net);
   co_pdo_trigger (net);
   return EVENT_NONE;
}

static co_fsm_event_t co_nmt_poweron (co_net_t * net, co_fsm_event_t event)
{
   co_lss_init (net);

   /* Copy persistent bitrate to pending/active bitrate */
   net->lss.bitrate = co_lss_get_persistent_bitrate (net);
   net->bitrate     = net->lss.bitrate;

   return EVENT_INITDONE;
}

/* State transitions, CiA 301 chapter 7.3.2.1 and 7.3.2.2 */
const co_fsm_transition_t transitions[] = {
   {STATE_OFF, EVENT_RESET, STATE_INIT_PWRON, co_nmt_poweron},
   {STATE_INIT_PWRON, EVENT_INITDONE, STATE_INIT_APP, co_nmt_reset_app},
   {STATE_INIT_APP, EVENT_INITDONE, STATE_INIT_COMM, co_nmt_reset_comm},
   {STATE_INIT_COMM, EVENT_INITDONE, STATE_PREOP, co_nmt_bootup},
   {STATE_PREOP, EVENT_START, STATE_OP, co_nmt_start},
   {STATE_OP, EVENT_PREOP, STATE_PREOP, NULL},
   {STATE_PREOP, EVENT_STOP, STATE_STOP, NULL},
   {STATE_STOP, EVENT_START, STATE_OP, co_nmt_start},
   {STATE_STOP, EVENT_PREOP, STATE_PREOP, NULL},
   {STATE_OP, EVENT_STOP, STATE_STOP, NULL},
   {STATE_OP, EVENT_RESET, STATE_INIT_APP, co_nmt_reset_app},
   {STATE_STOP, EVENT_RESET, STATE_INIT_APP, co_nmt_reset_app},
   {STATE_PREOP, EVENT_RESET, STATE_INIT_APP, co_nmt_reset_app},
   {STATE_OP, EVENT_RESETCOMM, STATE_INIT_COMM, co_nmt_reset_comm},
   {STATE_STOP, EVENT_RESETCOMM, STATE_INIT_COMM, co_nmt_reset_comm},
   {STATE_PREOP, EVENT_RESETCOMM, STATE_INIT_COMM, co_nmt_reset_comm},
};

const char * co_state_literals[] = {
   "STATE_OFF",
   "STATE_INIT_PWRON",
   "STATE_INIT_APP",
   "STATE_INIT_COMM",
   "STATE_PREOP",
   "STATE_OP",
   "STATE_STOP",
};

void co_nmt_event (co_net_t * net, co_fsm_event_t event)
{
   do
   {
      co_state_t previous = net->state;
      co_fsm_t * element  = &fsm[previous][event];

      /* Transition to next state */
      net->state = element->next;
      LOG_INFO (CO_NMT_LOG, "state = %s\n", co_state_literals[net->state]);

      /* Perform state action */
      event = (element->action) ? element->action (net, event) : EVENT_NONE;

      /* Call user callback if state has changed */
      if (previous != net->state)
      {
         if (net->cb_nmt)
         {
            net->cb_nmt (net, net->state);
         }
      }

   } while (event != EVENT_NONE);
}

void co_nmt_init (co_net_t * net)
{
   unsigned int i, j;

   /* Set FSM defaults */
   for (i = 0; i < STATE_LAST; i++)
   {
      for (j = 0; j < EVENT_LAST; j++)
      {
         /* Stay in state, no action */
         fsm[i][j].next   = i;
         fsm[i][j].action = NULL;
      }
   }

   /* Set FSM transitions from table */
   for (i = 0; i < NELEMENTS (transitions); i++)
   {
      const co_fsm_transition_t * t  = &transitions[i];
      fsm[t->state][t->event].next   = t->next;
      fsm[t->state][t->event].action = t->action;
   }

   /* Transition from OFF to INIT on poweron */
   net->state = STATE_OFF;
   co_nmt_event (net, EVENT_RESET);
}

int co_nmt_rx (co_net_t * net, uint32_t id, uint8_t * msg, size_t dlc)
{
   co_nmt_cmd_t cmd = msg[0];
   uint8_t node     = msg[1];
   co_fsm_event_t event;

   /* Check ID */
   if (id != 0)
      return -1;

   /* Check recipient */
   if (node != 0 && node != net->node)
      return -1;

   if (dlc != 2)
   {
      /* Ignore bad message */
      return -1;
   }

   switch (cmd)
   {
   case CO_NMT_OPERATIONAL:
      event = EVENT_START;
      break;
   case CO_NMT_STOPPED:
      event = EVENT_STOP;
      break;
   case CO_NMT_PRE_OPERATIONAL:
      event = EVENT_PREOP;
      break;
   case CO_NMT_RESET_NODE:
      event = EVENT_RESET;
      if (net->cb_reset)
      {
         net->cb_reset (net);
      }
      break;
   case CO_NMT_RESET_COMMUNICATION:
      event = EVENT_RESETCOMM;
      break;
   default:
      LOG_ERROR (CO_NMT_LOG, "bad nmt command %x\n", cmd);
      return -1;
   }

   /* Trigger FSM transition */
   co_nmt_event (net, event);

   return 0;
}
