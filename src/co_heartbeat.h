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
 * @file
 * @brief Handles heartbeat protocol
 */

#ifndef CO_HEARTBEAT_H
#define CO_HEARTBEAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_api.h"
#include "co_main.h"

/**
 * Receive heartbeat message
 *
 * This function should be called when a heartbeat message is
 * received. The heartbeat for the sending node will be updated.
 *
 *
 * @param net           network handle
 * @param node          Node ID
 * @param msg           CAN message
 * @param dlc           size of CAN message
 *
 * @return 0 always
 */
int co_heartbeat_rx (co_net_t * net, uint8_t node, void * msg, size_t dlc);

/**
 * Heartbeat timer
 *
 * This function performs the heartbeat producer and consumer
 * protocols and should be called periodically. An emergency will be
 * triggered if a consumed heartbeat has expired.
 *
 * @param net           network handle
 * @param now           current timestamp
 *
 * @return 0 on success, -1 on failure
 */
int co_heartbeat_timer (co_net_t * net, uint32_t now);

#ifdef __cplusplus
}
#endif

#endif /* CO_HEARTBEAT_H */
