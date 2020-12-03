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
 * @brief Handles synchronization object (SYNC)
 */

#ifndef CO_SYNC_H
#define CO_SYNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_api.h"
#include "co_main.h"

/**
 * SYNC timer
 *
 * This function performs the SYNC producer protocol and should be
 * called periodically.
 *
 * @param net           network handle
 * @param now           current timestamp
 *
 * @return 0 on success, -1 on failure
 */
int co_sync_timer (co_net_t * net, uint32_t now);

#ifdef __cplusplus
}
#endif

#endif /* CO_SYNC_H */
