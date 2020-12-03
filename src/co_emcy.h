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
 * @brief Handles emergency object (EMCY)
 */

#ifndef CO_EMCY_H
#define CO_EMCY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_main.h"

/**
 * Transmit emergency object (EMCY)
 *
 * This function transmits an emergency object. An optional
 * manufacturer-speficic error code can be included.
 *
 * Calling this function adds an error to the error history object
 * (1003h). It also signals an error to the NMT state-machine which
 * may change state according to the setting of the error behavior
 * object (1029h).
 *
 * The application will be notified via the EMCY callback. The node id
 * will be the active node id for this node.
 *
 * @param net           network handle
 * @param code          error code, lower 16 bits of error number
 * @param info          additional info, upper 16 bits of error number
 * @param msef          manufacturer-specific error code or NULL
 *
 * @return 0 always
 */
int co_emcy_tx (co_net_t * net, uint16_t code, uint16_t info, uint8_t msef[5]);

/**
 * Receive emergency object (EMCY)
 *
 * This function should be called when an emergency object is
 * received. The application will be notified via the EMCY callback
 * function, if the EMCY object is being consumed.
 *
 * @param net           network handle
 * @param id            CAN ID
 * @param msg           CAN message
 * @param dlc           size of CAN message
 *
 * @return 0 always
 */
int co_emcy_rx (co_net_t * net, uint32_t node, uint8_t * msg, size_t dlc);

/**
 * Handle CAN error states
 *
 * This function should be called periodically to check for CAN error
 * states. It generates emergencies for overrun, error passive mode
 * and bus off states.
 *
 * @param net           network handle
 */
void co_emcy_handle_can_state (co_net_t * net);

/**
 * Set error register bits
 *
 * This function sets bits in the CANopen error register. All bits can
 * be set, but note that the stack internally handles CO_ERR_GENERIC
 * and CO_ERR_COMMUNICATION. CO_ERR_GENERIC is set if any other bit is
 * set, and cleared otherwise. CO_ERR_COMMUNICATION is set if the
 * stack detects a communication problem.
 *
 * @param net           network handle
 * @param mask          bits to set
 */
void co_emcy_error_register_set (co_net_t * net, uint8_t mask);

/**
 * Clear error register bits
 *
 * @param net           network handle
 * @param mask          bits to clear
 */
void co_emcy_error_register_clear (co_net_t * net, uint8_t mask);

/**
 * Get error register value
 *
 * @param net           network handle
 *
 * @return error register value
 */
uint8_t co_emcy_error_register_get (co_net_t * net);

/**
 * Start emergency job
 *
 * @param net           network handle
 * @param job           emcy job
 */
void co_emcy_job (co_net_t * net, co_job_t * job);

#ifdef __cplusplus
}
#endif

#endif /* CO_EMCY_H */
