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

#ifndef COAL_CAN_H
#define COAL_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "coal_can_sys.h"

#ifndef OS_CHANNEL
typedef void os_channel_t;
#endif

typedef struct os_channel_state
{
   bool overrun;
   bool error_passive;
   bool bus_off;
} os_channel_state_t;

os_channel_t * os_channel_open (const char * name, void * callback, void * arg);
int os_channel_send (
   os_channel_t * channel,
   uint32_t id,
   const void * data,
   size_t dlc);
int os_channel_send_rtr (os_channel_t * channel, uint32_t id, size_t dlc);
int os_channel_receive (
   os_channel_t * channel,
   uint32_t * id,
   void * data,
   size_t * dlc);
int os_channel_set_bitrate (os_channel_t * channel, int bitrate);
int os_channel_set_filter (os_channel_t * channel, uint8_t * filter, size_t size);
int os_channel_bus_on (os_channel_t * channel);
int os_channel_bus_off (os_channel_t * channel);
int os_channel_get_state (os_channel_t * channel, os_channel_state_t * state);

#ifdef __cplusplus
}
#endif

#endif /* COAL_CAN_H */
