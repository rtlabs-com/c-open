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
 * @brief Handles NMT state-machine
 */

#ifndef CO_NMT_H
#define CO_NMT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_api.h"
#include "co_main.h"

/**
 * NMT state-machine events.
 *
 * NMT events trigger state transitions. See CiA 301 chapter 7.3.2.1.
 */
typedef enum co_fsm_event
{
   EVENT_NONE,
   EVENT_POWERON,
   EVENT_INITDONE,
   EVENT_START,
   EVENT_PREOP,
   EVENT_STOP,
   EVENT_RESET,
   EVENT_RESETCOMM,
   EVENT_LAST
} co_fsm_event_t;

/**
 * Trigger NMT state transition
 *
 * This function triggers an NMT state transition according to the
 * event.
 *
 * @param net           network handle
 * @param event         NMT event
 */
void co_nmt_event (co_net_t * net, co_fsm_event_t event);

/**
 * Receive NMT message
 *
 * This function handles the NMT protocols and should be called when
 * an NMT message is received. The NMT state-machine will change state
 * according to the message.
 *
 * @param net           network handle
 * @param id            CAN ID
 * @param msg           CAN message
 * @param dlc           size of CAN message
 *
 * @return 0 on success, -1 otherwise
 */
int co_nmt_rx (co_net_t * net, uint32_t id, uint8_t * msg, size_t dlc);

/**
 * Initialise NMT state-machine
 *
 * This function initialises the NMT state-machine and should be
 * called when the stack is started.
 *
 * @param net           network handle
 */
void co_nmt_init (co_net_t * net);

#ifdef __cplusplus
}
#endif

#endif /* CO_NMT_H */
