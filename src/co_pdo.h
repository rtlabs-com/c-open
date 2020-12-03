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
 * @brief Handles process data object (PDO)
 */

#ifndef CO_PDO_H
#define CO_PDO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_api.h"
#include "co_main.h"

/**
 * @internal
 * Pack PDO into CAN message
 *
 * This function packs a PDO into a CAN message for transmission. This
 * function is exported from the module to simplify unit-testing.
 *
 * @param net           network handle
 * @param pdo           PDO to pack
 */
void co_pdo_pack (co_net_t * net, co_pdo_t * pdo);

/**
 * @internal
 * Unpack PDO from CAN message
 *
 * This function unpacks a PDO from a received CAN message. This
 * function is exported from the module to simplify unit-testing.
 *
 * @param net           network handle
 * @param pdo           PDO to pack
 */
void co_pdo_unpack (co_net_t * net, co_pdo_t * pdo);

/**
 * @internal
 * Initialise PDO mapping
 *
 * This function is called internally on an NMT reset event to
 * initialise the PDO mapping. It should only be called internally.
 *
 * @param net           network handle
 * @param pdo           PDO to pack
 */
void co_pdo_mapping_init (co_net_t * net);

/**
 * Receive SYNC object
 *
 * This function handles synchronously triggered PDOs and should be
 * called when the SYNC object is received. Synchronous TPDOs will be
 * transmitted. Synchronous RPDOs will be delivered.
 *
 * @param net           network handle
 * @param msg           CAN message
 * @param dlc           size of CAN message
 *
 * @return 0 always
 */
int co_pdo_sync (co_net_t * net, uint8_t * msg, size_t dlc);

/**
 * Receive RPDO or remotely requested TPDO
 *
 * This function handles reception of RPDOs and remotely requested
 * TPDOs.
 *
 * @param net           network handle
 * @param id            CAN ID
 * @param msg           CAN message
 * @param dlc           size of CAN message
 */
void co_pdo_rx (co_net_t * net, uint32_t id, void * msg, size_t dlc);

/**
 * PDO timer
 *
 * This function handles TPDOs with event timer and should be called
 * periodically. The TPDO is transmitted if the timer has expired.
 *
 * @param net           network handle
 * @param now           current timestamp
 *
 * @return 0 on success, -1 on failure
 */
int co_pdo_timer (co_net_t * net, uint32_t now);

/**
 * PDO trigger
 *
 * This function triggers an event on event-driven and acyclic
 * TPDOs. Event-driven TPDOs will be transmitted immediately while
 * acyclic TPDOs will be queued for transmission at next SYNC.
 *
 * @param net           network handle
 */
void co_pdo_trigger (co_net_t * net);

/**
 * PDO trigger with object
 *
 * This function triggers an event on event-driven and acyclic
 * TPDOs that map the specified object. Event-driven TPDOs will be
 * transmitted immediately while acyclic TPDOs will be queued for
 * transmission at next SYNC.
 *
 * @param net           network handle
 * @param index         index
 * @param subindex      subindex
 */
void co_pdo_trigger_with_obj (co_net_t * net, uint16_t index, uint8_t subindex);

/**
 * Start PDO job
 *
 * @param net           network handle
 * @param job           emcy job
 */
void co_pdo_job (co_net_t * net, co_job_t * job);

/**
 * Initialise PDOs
 *
 * @param net           network handle
 *
 * @return 0 on success, -1 on failure
 */
int co_pdo_init (co_net_t * net);

#ifdef __cplusplus
}
#endif

#endif /* CO_PDO_H */
