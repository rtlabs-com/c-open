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
 * @brief Support routines for rt-kernel
 */
#ifndef CO_RTK_H
#define CO_RTK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <can/can.h>

/**
 * Get can configuration for bitrate
 *
 * This function returns the can configuration parameters for the
 * given bitrate.
 *
 * @param bitrate       CAN bitrate (bits per second)
 * @param cfg           CAN configuration parameters
 */
void co_can_get_cfg (int bitrate, can_cfg_t * cfg);

#ifdef __cplusplus
}
#endif

#endif /* CO_RTK_H */
