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

#include "osal_can.h"
#include "osal.h"
#include "options.h"
#include "log.h"
#include "co_log.h"
#include "co_main.h"
#include "co_rtk.h"
#include "cc.h"

#include <can/can.h>
#include <fcntl.h>
#include <string.h>

static void os_can_callback (void * arg, can_event_t event)
{
   os_channel_t * channel = arg;

   if (channel->callback)
      channel->callback (channel->arg);
}

os_channel_t * os_channel_open (const char * name, void * callback, void * arg)
{
   os_channel_t * channel = malloc (sizeof(*channel));
   can_filter_t filter = { .id = 0xFFFFFFFF, .mask = 0, };

   /* Create file descriptor for CAN bus */
   channel->handle = open (name, O_RDWR);
   if (channel->handle < 0)
   {
      free (channel);
      return NULL;
   }

   channel->callback = callback;
   channel->arg = arg;

   can_set_callback(channel->handle, os_can_callback,
                    CAN_EVENT_MSG_RECEIVED, channel);

   can_filter (channel->handle, &filter);

   return channel;
}

int os_channel_send (os_channel_t * channel, uint32_t id, const void * data, size_t dlc)
{
   can_frame_t frame;

   co_msg_log ("Tx", id, data, dlc);

   frame.id = id & CO_ID_MASK;
   frame.id |= (id & CO_RTR_MASK) ? CAN_ID_RTR : 0;
   frame.id |= (id & CO_EXT_MASK) ? CAN_ID_EXT : 0;
   frame.id = id;
   frame.dlc = dlc;
   memcpy (frame.data, data, dlc);

   can_transmit (channel->handle, &frame);

   return 0;
}

int os_channel_receive (os_channel_t * channel, uint32_t * id, void * data, size_t * dlc)
{
   can_frame_t frame;
   int result;

   result = can_receive (channel->handle, &frame);
   if (result != 0)
      return -1;

   *id = frame.id & CAN_ID_MASK;
   *id |= (frame.id & CAN_ID_RTR) ? CO_RTR_MASK : 0;
   *id |= (frame.id & CAN_ID_EXT) ? CO_EXT_MASK : 0;
   *dlc = frame.dlc;
   memcpy (data, frame.data, frame.dlc);

   co_msg_log ("Rx", *id, data, *dlc);

   return 0;
}

CC_ATTRIBUTE_WEAK void co_can_get_cfg (int bitrate, can_cfg_t * cfg)
{
   ASSERT (0);
}

int os_channel_set_bitrate (os_channel_t * channel, int bitrate)
{
   can_cfg_t cfg;

   /* Get can parameters from application */
   co_can_get_cfg (bitrate, &cfg);

   /* Set can parameters */
   can_set_cfg (channel->handle, &cfg);

   return 0;
}

int os_channel_set_filter (os_channel_t * channel, uint8_t * filter, size_t size)
{
   return 0;
}

int os_channel_bus_on (os_channel_t * channel)
{
   can_bus_on (channel->handle);
   return 0;
}

int os_channel_bus_off (os_channel_t * channel)
{
   can_bus_off (channel->handle);
   return 0;
}

int os_channel_get_state (os_channel_t * channel, os_channel_state_t * state)
{
   can_status_t can_status;

   can_get_status (channel->handle, &can_status);

   state->overrun       = can_status.flags & CAN_STATUS_BUFFER_OVERFLOW;
   state->error_passive = can_status.flags & CAN_STATUS_ERROR_PASSIVE;
   state->bus_off       = can_status.flags & CAN_STATUS_BUS_OFF;

   return 0;
}
