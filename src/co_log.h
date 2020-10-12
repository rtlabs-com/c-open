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
 * @brief CAN message logging utilities
 */

#ifndef CO_LOG_H
#define CO_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

/**
 * Log CAN message
 *
 * This function logs CAN messages.
 *
 * @param prefix        Prefix of message
 * @param id            CAN ID
 * @param data          CAN message
 * @param dlc           size of CAN message
 */
void co_msg_log (char * prefix, uint32_t id, const uint8_t * data, size_t dlc);

#ifdef __cplusplus
}
#endif

#endif /* CO_LOG_H */
