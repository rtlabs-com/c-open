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

#ifndef OSAL_SYS_H
#define OSAL_SYS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <kern.h>

#define OS_THREAD
#define OS_MUTEX
#define OS_SEM
#define OS_EVENT
#define OS_MBOX
#define OS_TIMER

typedef task_t os_thread_t;
typedef mtx_t os_mutex_t;
typedef sem_t os_sem_t;
typedef flags_t os_event_t;
typedef mbox_t os_mbox_t;
typedef tmr_t os_timer_t;

#ifdef __cplusplus
}
#endif

#endif /* OSAL_SYS_H */
