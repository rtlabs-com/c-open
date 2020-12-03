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
 * @brief Handles node guarding protocol
 */

#ifndef CO_NODE_GUARD_H
#define CO_NODE_GUARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_api.h"
#include "co_main.h"

/**
 * Receive node guard message
 *
 * This function should be called when a node guard message is
 * received. The node guard timestamp will be updated.
 *
 * @param net           network handle
 * @param id            CAN ID
 * @param msg           CAN message
 * @param dlc           size of CAN message
 *
 * @return 0 always
 */
int co_node_guard_rx (co_net_t * net, uint32_t id, void * msg, size_t dlc);

/**
 * Node guard timer
 *
 * This function performs the node guard protocol and should be called
 * periodically. An emergency will be triggered if node guarding has
 * expired.
 *
 * @param net           network handle
 * @param now           current timestamp
 *
 * @return 0 on success, -1 on failure
 */
int co_node_guard_timer (co_net_t * net, uint32_t now);

#ifdef __cplusplus
}
#endif

#endif /* CO_NODE_GUARD_H */
