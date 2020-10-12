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

#include "osal.h"
#include "options.h"
#include "log.h"
#include "co_log.h"
#include "co_main.h"

#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

#include <canlib.h>

static void CANLIBAPI
os_can_callback (CanHandle handle, void * context, unsigned int event)
{
   os_channel_t * channel = context;

   if (channel->callback)
      channel->callback (channel->arg);
}

os_channel_t * os_channel_open (const char * name, void * callback, void * arg)
{
   os_channel_t * channel = malloc (sizeof (*channel));
   unsigned long ix       = strtoul (name, NULL, 0);

   canInitializeLibrary();

   channel->handle = canOpenChannel (ix, 0);
   if (channel->handle < 0)
   {
      free (channel);
      return NULL;
   }

   channel->callback = callback;
   channel->arg      = arg;

   kvSetNotifyCallback (channel->handle, os_can_callback, channel, canNOTIFY_RX);

   return channel;
}

int os_channel_send (os_channel_t * channel, uint32_t id, const void * data, size_t dlc)
{
   canStatus status;
   uint32_t flags = 0;

   co_msg_log ("Tx", id, data, dlc);

   flags |= (id & CO_RTR_MASK) ? canMSG_RTR : 0;
   flags |= (id & CO_EXT_MASK) ? canMSG_EXT : 0;

   status = canWrite (channel->handle, id & CO_ID_MASK, (void *)data, dlc, flags);
   if (status < canOK)
      return status;

   return 0;
}

int os_channel_receive (
   os_channel_t * channel,
   uint32_t * id,
   void * data,
   size_t * dlc)
{
   canStatus status;
   unsigned int flags;

   status = canRead (channel->handle, id, data, dlc, &flags, NULL);
   if (status < canOK)
      return status;

   *id |= (flags & canMSG_RTR) ? CO_RTR_MASK : 0;
   *id |= (flags & canMSG_EXT) ? CO_EXT_MASK : 0;

   co_msg_log ("Rx", *id, data, *dlc);

   return 0;
}

int os_channel_set_bitrate (os_channel_t * channel, int bitrate)
{
   canStatus status;

   switch (bitrate)
   {
   case 100 * 1000:
      bitrate = canBITRATE_100K;
      break;
   case 125 * 1000:
      bitrate = canBITRATE_125K;
      break;
   case 250 * 1000:
      bitrate = canBITRATE_250K;
      break;
   case 500 * 1000:
      bitrate = canBITRATE_500K;
      break;
   case 1000 * 1000:
      bitrate = canBITRATE_1M;
      break;
   default:
      assert (0);
   }

   status = canSetBusParams (channel->handle, bitrate, 0, 0, 0, 0, 0);
   if (status < canOK)
      return status;

   return 0;
}

int os_channel_set_filter (os_channel_t * channel, uint8_t * filter, size_t size)
{
   return 0;
}

int os_channel_bus_on (os_channel_t * channel)
{
   canStatus status;

   status = canBusOn (channel->handle);
   if (status < canOK)
      return status;

   return 0;
}

int os_channel_bus_off (os_channel_t * channel)
{
   canStatus status;

   status = canBusOff (channel->handle);
   if (status < canOK)
      return status;

   return 0;
}

int os_channel_get_state (os_channel_t * channel, os_channel_state_t * state)
{
   canStatus status;
   unsigned long flags;

   status = canReadStatus (channel->handle, &flags);
   if (status < canOK)
      return status;

   state->overrun       = flags & canSTAT_OVERRUN;
   state->error_passive = flags & canSTAT_ERROR_PASSIVE;
   state->bus_off       = flags & canSTAT_BUS_OFF;

   return 0;
}
