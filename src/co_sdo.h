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
 * @brief Handles service data object (SDO)
 */

#ifndef CO_SDO_H
#define CO_SDO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_api.h"
#include "co_main.h"

#define CO_SDO_xCS(v) ((v)&0xE0)
#define CO_SDO_N(v)   (((v) >> 2) & 0x03)
#define CO_SDO_E      BIT (1)
#define CO_SDO_S      BIT (0)

#define CO_SDO_CCS_DOWNLOAD_SEG_REQ  (0 << 5)
#define CO_SDO_CCS_DOWNLOAD_INIT_REQ (1 << 5)
#define CO_SDO_CCS_UPLOAD_INIT_REQ   (2 << 5)
#define CO_SDO_CCS_UPLOAD_SEG_REQ    (3 << 5)

#define CO_SDO_SCS_UPLOAD_SEG_RSP    (0 << 5)
#define CO_SDO_SCS_DOWNLOAD_SEG_RSP  (1 << 5)
#define CO_SDO_SCS_UPLOAD_INIT_RSP   (2 << 5)
#define CO_SDO_SCS_DOWNLOAD_INIT_RSP (3 << 5)

#define CO_SDO_xCS_ABORT (4 << 5)

#define CO_SDO_TOGGLE   BIT (4)
#define CO_SDO_N_SEG(v) (((v) >> 1) & 0x07)
#define CO_SDO_C        BIT (0)

#define CO_SDO_INDEX(d)    (d[1] << 8 | d[2])
#define CO_SDO_SUBINDEX(d) (d[3])

/**
 * Send SDO abort message
 *
 * This function sends an SDO abort message.
 *
 * @param net           network handle
 * @param id            COB ID
 * @param index         failed index
 * @param subindex      failed subindex
 * @param code          abort code
 */
void co_sdo_abort (
   co_net_t * net,
   uint16_t id,
   uint16_t index,
   uint8_t subindex,
   uint32_t code);

/**
 * @internal
 * SDO toggle protocol
 *
 * This function handles the SDO toggle protocol.
 *
 * @param job           job descriptor
 * @param type          transfer type (first byte of message)
 */
int co_sdo_toggle_update (co_job_t * job, uint8_t type);

/**
 * Receive SDO TX message
 *
 * This function should be called when an SDO TX message is
 * received. The SDO client will process the message. TODO: rename to
 * sdo_client_rx?
 *
 * @param net           network handle
 * @param node          Node ID
 * @param msg           CAN message
 * @param dlc           size of CAN message
 *
 * @return 0 always
 */
int co_sdo_tx (co_net_t * net, uint8_t node, void * msg, size_t dlc);

/**
 * Receive SDO RX message
 *
 * This function should be called when an SDO RX message is
 * received. The SDO server will process the message. TODO: rename to
 * sdo_server_rx?
 *
 * @param net           network handle
 * @param node          Node ID
 * @param msg           CAN message
 * @param dlc           size of CAN message
 *
 * @return 0 always
 */
int co_sdo_rx (co_net_t * net, uint8_t node, void * msg, size_t dlc);

/**
 * SDO server timer
 *
 * This function checks for SDO server timeouts and should be called
 * periodically. An SDO abort will be triggered if the timeout has
 * expired.
 *
 * @param net           network handle
 * @param now           current timestamp
 *
 * @return 0 on success, -1 on failure
 */
int co_sdo_server_timer (co_net_t * net, uint32_t now);

/**
 * SDO client timer
 *
 * This function checks for SDO client timeouts and should be called
 * periodically. An SDO abort will be triggered if the timeout has
 * expired.
 *
 * @param net           network handle
 * @param now           current timestamp
 *
 * @return 0 on success, -1 on failure
 */
int co_sdo_client_timer (co_net_t * net, uint32_t now);

/**
 * Issue SDO request
 *
 * This function issues an SDO request. It used to start an SDO client
 * transfer. The SDO parameters are specified in the SDO job.
 *
 * @param net           network handle
 * @param job           SDO job
 */
void co_sdo_issue (co_net_t * net, co_job_t * job);

#ifdef __cplusplus
}
#endif

#endif /* CO_SDO_H */
