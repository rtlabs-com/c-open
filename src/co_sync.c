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
#define os_channel_send mock_os_channel_send
#endif

#include "co_sync.h"
#include "co_pdo.h"
#include "co_sdo.h"
#include "co_util.h"

uint32_t co_od1005_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   co_sync_t * sync = &net->sync;

   switch (event)
   {
   case OD_EVENT_READ:
      *value = sync->cobid;
      return 0;

   case OD_EVENT_WRITE:
      if (!co_validate_cob_id (*value))
         return CO_SDO_ABORT_VALUE;

      if ((*value & sync->cobid & BIT (30)) == 0)
      {
         sync->cobid     = *value;
         sync->timestamp = os_get_current_time_us();
         return 0;
      }

      return CO_SDO_ABORT_GENERAL;

   case OD_EVENT_RESTORE:
      sync->cobid = 0x80;
      return 0;

   default:
      return CO_SDO_ABORT_GENERAL;
   }
   return 0;
}

uint32_t co_od1006_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   co_sync_t * sync = &net->sync;

   switch (event)
   {
   case OD_EVENT_READ:
      *value = sync->period;
      return 0;

   case OD_EVENT_WRITE:
      sync->period = *value;
      return 0;

   case OD_EVENT_RESTORE:
      sync->period  = 0;
      sync->counter = 1;
      return 0;

   default:
      return CO_SDO_ABORT_GENERAL;
   }
   return 0;
}

uint32_t co_od1019_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   co_sync_t * sync = &net->sync;

   switch (event)
   {
   case OD_EVENT_READ:
      *value = sync->overflow;
      return 0;

   case OD_EVENT_WRITE:
      if (net->state == STATE_INIT)
      {
         sync->overflow = *value;
         return 0;
      }

      if (sync->period != 0)
         return CO_SDO_ABORT_WRITE_STATE_DENIED;

      if (*value == 1 || *value > 240)
         return CO_SDO_ABORT_VALUE;

      sync->overflow  = *value;
      sync->timestamp = os_get_current_time_us();
      return 0;

   case OD_EVENT_RESTORE:
      sync->overflow = 0;
      return 0;

   default:
      return CO_SDO_ABORT_GENERAL;
   }
   return 0;
}

int co_sync_timer (co_net_t * net, uint32_t now)
{
   co_sync_t * sync = &net->sync;

   if (net->state != STATE_PREOP && net->state != STATE_OP)
      return -1;

   if ((sync->cobid & BIT (30)) == 0)
      return -1;

   /* Sync producer */
   if (sync->period != 0)
   {
      if (co_is_expired (now, sync->timestamp, sync->period))
      {
         sync->timestamp = now;

         if (sync->overflow)
         {
            uint8_t msg[1];
            co_put_uint8 (msg, sync->counter);
            os_channel_send (
               net->channel,
               sync->cobid & CO_EXTID_MASK,
               msg,
               sizeof (msg));
            co_pdo_sync (net, msg, sizeof (msg));

            if (sync->counter++ == sync->overflow)
               sync->counter = 1;
         }
         else
         {
            os_channel_send (net->channel, sync->cobid & CO_EXTID_MASK, NULL, 0);
            co_pdo_sync (net, NULL, 0);
         }

         /* Call user callback */
         if (net->cb_sync)
         {
            net->cb_sync (net);
         }
      }
   }

   return 0;
}
