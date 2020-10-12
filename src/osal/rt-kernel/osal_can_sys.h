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

#ifndef OSAL_CAN_SYS_H
#define OSAL_CAN_SYS_H

#ifdef __cplusplus
extern "C" {
#endif

#define OS_CHANNEL

typedef struct
{
   int handle;
   void (*callback) (void * arg);
   void * arg;
} os_channel_t;

#ifdef __cplusplus
}
#endif

#endif /* OSAL_CAN_SYS_H */
