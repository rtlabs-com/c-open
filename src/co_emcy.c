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
#define os_channel_get_state   mock_os_channel_get_state
#define os_channel_bus_on      mock_os_channel_bus_on
#define os_get_current_time_us mock_os_get_current_time_us
#endif

#include "co_emcy.h"
#include "co_nmt.h"
#include "co_sdo.h"
#include "co_util.h"

#include <string.h>

static uint32_t co_emcy_error_get (co_net_t * net, uint8_t subindex, uint32_t * value)
{
   uint8_t ix;

   if (subindex == 0)
   {
      *value = net->number_of_errors;
      return 0;
   }

   /* Get error from list */
   ix = subindex - 1;
   if (ix < net->number_of_errors)
   {
      *value = net->errors[ix];
      return 0;
   }

   return CO_SDO_ABORT_NO_DATA;
}

static uint32_t co_emcy_error_set (co_net_t * net, uint8_t subindex, uint32_t * value)
{
   /* Only subindex 0 is writable. Assert that this function is only
      called for subindex 0. */
   CC_ASSERT (subindex == 0);

   if (*value != 0)
      return CO_SDO_ABORT_VALUE;

   net->number_of_errors = 0;
   return 0;
}

static void co_trigger_error_behavior (co_net_t * net)
{
   /* Transition state according to error behavior setting */
   switch (net->error_behavior)
   {
   case 0:
      if (net->state == STATE_OP)
         co_nmt_event (net, EVENT_PREOP);
      break;
   case 2:
      co_nmt_event (net, EVENT_STOP);
      break;
   default:
      /* Do nothing */
      break;
   }
}

uint32_t co_od1001_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (event == OD_EVENT_READ)
      *value = co_emcy_error_register_get (net);
   return 0;
}

uint32_t co_od1003_fn (
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
      return co_emcy_error_get (net, subindex, value);
   case OD_EVENT_WRITE:
      return co_emcy_error_set (net, subindex, value);
   case OD_EVENT_RESTORE:
      net->number_of_errors = 0;
      return 0;
   default:
      return CO_SDO_ABORT_GENERAL;
   }
}

uint32_t co_od1014_fn (
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
      *value = net->emcy.cobid;
      return 0;
   case OD_EVENT_WRITE:
      if (!co_validate_cob_id (*value))
         return CO_SDO_ABORT_VALUE;
      net->emcy.cobid = *value;
      return 0;
   case OD_EVENT_RESTORE:
      net->emcy.cobid = net->node + 0x80;
      return 0;
   default:
      return CO_SDO_ABORT_GENERAL;
   }
}

uint32_t co_od1015_fn (
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
      *value = net->emcy.inhibit;
      return 0;
   case OD_EVENT_WRITE:
      net->emcy.inhibit = *value;
      return 0;
   case OD_EVENT_RESTORE:
      net->emcy.inhibit = 0;
      return 0;
   default:
      return CO_SDO_ABORT_GENERAL;
   }
}

uint32_t co_od1028_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   uint32_t cobid;

   if (subindex == 0 && event != OD_EVENT_RESTORE)
      return CO_SDO_ABORT_BAD_SUBINDEX;

   switch (event)
   {
   case OD_EVENT_READ:
      *value = net->emcy.cobids[subindex - 1];
      return 0;
   case OD_EVENT_WRITE:
      cobid = net->emcy.cobids[subindex - 1];
      if (((cobid | *value) & CO_COBID_INVALID) == 0)
         return CO_SDO_ABORT_VALUE;
      net->emcy.cobids[subindex - 1] = *value;
      return 0;
   case OD_EVENT_RESTORE:
      for (int ix = 0; ix < MAX_EMCY_COBIDS; ix++)
         net->emcy.cobids[ix] = CO_COBID_INVALID;
      return 0;
   default:
      return CO_SDO_ABORT_GENERAL;
   }

   return 0;
}

uint32_t co_od1029_fn (
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
      if (subindex == 0)
         return CO_SDO_ABORT_BAD_SUBINDEX;
      *value = net->error_behavior;
      return 0;
   case OD_EVENT_WRITE:
      if (subindex == 0)
         return CO_SDO_ABORT_BAD_SUBINDEX;
      net->error_behavior = (uint8_t)*value;
      return 0;
   case OD_EVENT_RESTORE:
      net->error_behavior = 0;
      return 0;
   default:
      return CO_SDO_ABORT_GENERAL;
   }
}

int co_emcy_tx (co_net_t * net, uint16_t code, uint16_t info, uint8_t msef[5])
{
   uint8_t msg[8] = {0};
   uint8_t * p    = msg;
   uint8_t reg;
   uint32_t now;
   bool error_behavior = false;

   if (net->number_of_errors < MAX_ERRORS)
      net->number_of_errors++;

   /* Move down previous errors. The oldest error is discarded. Note
      that memmove supports overlapping copy. */
   memmove (
      &net->errors[1],
      &net->errors[0],
      (net->number_of_errors - 1) * sizeof (net->errors[0]));

   net->errors[0] = info << 16 | code;

   reg = co_emcy_error_register_get (net);

   p = co_put_uint16 (p, code);
   p = co_put_uint8 (p, reg);

   if (msef != NULL)
   {
      p = co_put_uint8 (p, msef[0]);
      p = co_put_uint8 (p, msef[1]);
      p = co_put_uint8 (p, msef[2]);
      p = co_put_uint8 (p, msef[3]);
      p = co_put_uint8 (p, msef[4]);
      (void)p;
   }

   /* Send EMCY if inhibit time has expired */
   now = os_get_current_time_us();
   if (co_is_expired (now, net->emcy.timestamp, 100 * net->emcy.inhibit))
   {
      LOG_ERROR (CO_EMCY_LOG, "emcy %x\n", code);
      os_channel_send (net->channel, net->emcy.cobid, msg, sizeof (msg));
      net->emcy.timestamp = now;
   }

   /* Call user callback, except for bus-off recovery, where it was
    * called at the actual bus-off event. */
   if (net->cb_emcy && code != 0x8140)
   {
      error_behavior = net->cb_emcy (net, net->node, code, reg, msef);
   }

   /* Always trigger error behavior on the mandatory events,
    * otherwise, follow the callback return value. The bus-off
    * event was handled when it happened. */
   if (code == 0x8130 || error_behavior) {
      co_trigger_error_behavior (net);
   }

   return 0;
}

int co_emcy_rx (co_net_t * net, uint32_t id, uint8_t * msg, size_t dlc)
{
   uint16_t code;
   uint8_t reg;
   uint8_t msef[5];
   int ix;

   for (ix = 0; ix < MAX_EMCY_COBIDS; ix++)
   {
      uint32_t cobid = net->emcy.cobids[ix];

      /* Check for matching COB ID */
      if (cobid != id)
         continue;

      /* Consume this EMCY */
      code    = co_fetch_uint16 (&msg[0]);
      reg     = co_fetch_uint8 (&msg[2]);
      msef[0] = co_fetch_uint8 (&msg[3]);
      msef[1] = co_fetch_uint8 (&msg[4]);
      msef[2] = co_fetch_uint8 (&msg[5]);
      msef[3] = co_fetch_uint8 (&msg[6]);
      msef[4] = co_fetch_uint8 (&msg[7]);

      /* Call user callback */
      if (net->cb_emcy)
      {
         net->cb_emcy (net, CO_NODE_GET (id), code, reg, msef);
      }
   }

   return 0;
}

void co_emcy_handle_can_state (co_net_t * net)
{
   int status;
   uint32_t now = os_get_current_time_us();;
   os_channel_state_t previous = net->emcy.state;

   /* Get current state */
   status = os_channel_get_state (net->channel, &net->emcy.state);
   if (status != 0)
      return;

   /* Check for new communication errors */

   if (net->emcy.state.overrun && !previous.overrun)
   {
      /* CAN overrun */
      co_emcy_error_register_set (net, CO_ERR_COMMUNICATION);
      co_emcy_tx (net, 0x8110, 0, NULL);
   }

   if (net->emcy.state.error_passive && !previous.error_passive)
   {
      /* CAN in error passive mode */
      co_emcy_error_register_set (net, CO_ERR_COMMUNICATION);
      co_emcy_tx (net, 0x8120, 0, NULL);
   }

   if (net->emcy.state.bus_off && !previous.bus_off)
   {
      /* Entered bus off */
      co_emcy_error_register_set (net, CO_ERR_COMMUNICATION);
      net->emcy.bus_off_timestamp = now;

      /* Call user callback directly, cannot call co_emcy_tx() now */
      if (net->cb_emcy)
      {
         net->cb_emcy (net, net->node, 0x8140,
                       co_emcy_error_register_get(net), NULL);
      }

      co_trigger_error_behavior (net);
   }

   if (!net->emcy.state.bus_off && previous.bus_off)
   {
      /* Recovered from bus off */
      co_emcy_tx (net, 0x8140, 0, NULL);
   }

   /* Attempt to go bus on again. */
   if (net->emcy.state.bus_off && net->restart_ms > 0 &&
      co_is_expired (now, net->emcy.bus_off_timestamp, 1000 * net->restart_ms))
   {
      os_channel_bus_on(net->channel);
      net->emcy.bus_off_timestamp = now;
   }

   /* Clear communication error state if all sub-errors are inactive */
   if (
      !net->emcy.state.overrun && !net->emcy.state.error_passive &&
      !net->emcy.state.bus_off && !net->emcy.node_guard_error &&
      !net->emcy.heartbeat_error)
   {
      co_emcy_error_register_clear (net, CO_ERR_COMMUNICATION);
   }
}

void co_emcy_error_register_set (co_net_t * net, uint8_t mask)
{
   net->emcy.error |= mask;
}

void co_emcy_error_register_clear (co_net_t * net, uint8_t mask)
{
   uint8_t previous = net->emcy.error;

   net->emcy.error &= ~mask;

   if (previous & mask)
   {
      /* Notify that an error was reset */
      co_emcy_tx (net, 0, 0, NULL);
   }
}

uint8_t co_emcy_error_register_get (co_net_t * net)
{
   uint8_t value = net->emcy.error;

   /* Set generic error if any other error is active */
   if (value != 0)
      value |= CO_ERR_GENERIC;

   return value;
}

void co_emcy_job (co_net_t * net, co_job_t * job)
{
   switch (job->type)
   {
   case CO_JOB_EMCY_TX:
      co_emcy_tx (net, job->emcy.code, job->emcy.info, job->emcy.msef);
      break;
   case CO_JOB_ERROR_SET:
      co_emcy_error_register_set (net, job->emcy.value);
      break;
   case CO_JOB_ERROR_CLEAR:
      co_emcy_error_register_clear (net, job->emcy.value);
      break;
   case CO_JOB_ERROR_GET:
      job->emcy.value = co_emcy_error_register_get (net);
      break;
   default:
      CC_ASSERT (0);
   }

   job->result = 0;
   if (job->callback)
      job->callback (job);
}
