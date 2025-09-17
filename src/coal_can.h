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

/**
 * CAN abstraction layer
 */

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

/** Channel state */
typedef struct os_channel_state
{
   bool overrun;
   bool error_passive;
   bool bus_off;
} os_channel_state_t;

/**
 * Open channel to CAN bus
 *
 * \param name          Name of CAN channel
 * \param callback      Callback function called upon frame reception
 * \param arg           Argument passed to callback function
 * \return              Channel object pointer on success, NULL failure
 */
os_channel_t * os_channel_open (const char * name, void * callback, void * arg);

/**
 * Send CAN frame
 *
 * \param channel       CAN channel
 * \param id            CAN identifier
 * \param data          Data to be sent
 * \param dlc           Data Length Code
 * \return              0 on success, otherwise failure
 */
int os_channel_send (
   os_channel_t * channel,
   uint32_t id,
   const void * data,
   size_t dlc);

/**
 * Receive CAN frame
 *
 * \param channel       CAN channel
 * \param id            Received CAN identifier
 * \param data          Buffer to receive data
 * \param dlc           Returned Data Length Code
 * \return              0 on success, otherwise failure
 */
int os_channel_receive (
   os_channel_t * channel,
   uint32_t * id,
   void * data,
   size_t * dlc);

/**
 * Set CAN bus bit rate
 *
 * \param channel       CAN channel
 * \param bitrate       Data bit rate in bits per second
 * \return              0 on success, otherwise failure
 */
int os_channel_set_bitrate (os_channel_t * channel, int bitrate);

/**
 * Set reception filter
 *
 * \param channel       CAN channel
 * \param filter        Filter. Not used
 * \param size          Size. Note used
 * \return              0 on success, otherwise failure
 */
int os_channel_set_filter (os_channel_t * channel, uint8_t * filter, size_t size);

/**
 * Go on CAN bus
 *
 * \param channel       CAN channel
 * \return              0 on success, otherwise failure
 */
int os_channel_bus_on (os_channel_t * channel);

/**
 * Go off CAN bus
 *
 * \param channel       CAN channel
 * \return              0 on success, otherwise failure
 */
int os_channel_bus_off (os_channel_t * channel);

/**
 * Get current state of CAN bus
 *
 * \param channel       CAN channel
 * \param state         Reported state of CAN bus
 * \return              0 on success, otherwise failure
 */
int os_channel_get_state (os_channel_t * channel, os_channel_state_t * state);

#ifdef __cplusplus
}
#endif

#endif /* COAL_CAN_H */
