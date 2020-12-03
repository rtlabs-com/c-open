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
 * @brief Handle LSS protocol
 */

#ifndef CO_LSS_H
#define CO_LSS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_api.h"
#include "co_main.h"

/**
 * Get LSS persistent node ID
 *
 * This function returns the persistent (configured) node ID.
 *
 * @param net           network handle
 *
 * @return persistent node ID
 */
uint8_t co_lss_get_persistent_node_id (co_net_t * net);

/**
 * Get LSS persistent bitrate
 *
 * This function returns the persistent (configured) bitrate.
 *
 * @param net           network handle
 *
 * @return persistent bitrate
 */
int co_lss_get_persistent_bitrate (co_net_t * net);

/**
 * Receive LSS message
 *
 * This function handles the LSS protocol and should be called when an
 * LSS message is received.
 *
 * @param net           network handle
 * @param id            CAN ID
 * @param msg           CAN message
 * @param dlc           size of CAN message
 *
 * @return 0 on success, -1 otherwise
 */
int co_lss_rx (co_net_t * net, uint32_t id, uint8_t * msg, size_t dlc);

/**
 * Initialise LSS state
 *
 * This function initialises the LSS state and should be called when
 * the stack is started. TODO: From NMT poweron?
 *
 * @param net           network handle
 * @param id            CAN ID
 * @param msg           CAN message
 * @param dlc           size of CAN message
 */
void co_lss_init (co_net_t * net);

#ifdef __cplusplus
}
#endif

#endif /* CO_LSS_H */
