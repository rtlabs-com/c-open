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
extern "C"
{
#endif

#include "co_api.h"
#include "co_main.h"


#define CO_SDO_xCS(v)      ((v) & 0xE0)
#define CO_SDO_N(v)        (((v) >> 2) & 0x03)
#define CO_SDO_E           BIT (1)
#define CO_SDO_S           BIT (0)

#define CO_SDO_CCS_DOWNLOAD_SEG_REQ  (0 << 5)
#define CO_SDO_CCS_DOWNLOAD_INIT_REQ (1 << 5)
#define CO_SDO_CCS_UPLOAD_INIT_REQ   (2 << 5)
#define CO_SDO_CCS_UPLOAD_SEG_REQ    (3 << 5)

#define CO_SDO_SCS_UPLOAD_SEG_RSP    (0 << 5)
#define CO_SDO_SCS_DOWNLOAD_SEG_RSP  (1 << 5)
#define CO_SDO_SCS_UPLOAD_INIT_RSP   (2 << 5)
#define CO_SDO_SCS_DOWNLOAD_INIT_RSP (3 << 5)

#define CO_SDO_xCS_ABORT             (4 << 5)

#define CO_SDO_TOGGLE      BIT (4)
#define CO_SDO_N_SEG(v)    (((v) >> 1) & 0x07)
#define CO_SDO_C           BIT (0)

#define CO_SDO_INDEX(d)    (d[1] << 8 | d[2])
#define CO_SDO_SUBINDEX(d) (d[3])

/* Abort error codes */
typedef enum co_sdo_abort
{
   CO_SDO_ABORT_TOGGLE                = 0x05030000, /* Toggle bit not alternated. */
   CO_SDO_ABORT_TIMEOUT               = 0x05040000, /* SDO protocol timed out. */
   CO_SDO_ABORT_UNKNOWN               = 0x05040001, /* Client/server command specifier not valid or unknown. */
   CO_SDO_ABORT_INVALID_BLOCK_SIZE    = 0x05040002, /* Invalid block size (block mode only). */
   CO_SDO_ABORT_INVALID_SEQ_NO        = 0x05040003, /* Invalid sequence number (block mode only). */
   CO_SDO_ABORT_CRC_ERROR             = 0x05040004, /* CRC error (block mode only). */
   CO_SDO_ABORT_OUT_OF_MEMORY         = 0x05040005, /* Out of memory. */
   CO_SDO_ABORT_ACCESS                = 0x06010000, /* Unsupported access to an object. */
   CO_SDO_ABORT_ACCESS_WO             = 0x06010001, /* Attempt to read a write only object. */
   CO_SDO_ABORT_ACCESS_RO             = 0x06010002, /* Attempt to write a read only object. */
   CO_SDO_ABORT_BAD_INDEX             = 0x06020000, /* Object does not exist in the object dictionary. */
   CO_SDO_ABORT_UNMAPPABLE            = 0x06040041, /* Object cannot be mapped to the PDO. */
   CO_SDO_ABORT_PDO_LENGTH            = 0x06040042, /* The number and length of the objects to be
                                                       mapped would exceed PDO length. */
   CO_SDO_ABORT_PARAM_INCOMPATIBLE    = 0x06040043, /* General parameter incompatibility reason. */
   CO_SDO_ABORT_INTERNAL_INCOMPATIBLE = 0x06040047, /* General internal incompatibility in the device. */
   CO_SDO_ABORT_HW_ERROR              = 0x06060000, /* Access failed due to a hardware error. */
   CO_SDO_ABORT_LENGTH                = 0x06070010, /* Data type does not match, length of service
                                                       parameter does not match */
   CO_SDO_ABORT_LENGTH_TOO_HIGH       = 0x06070012, /* Data type does not match, length of service
                                                       parameter too high */
   CO_SDO_ABORT_LENGTH_TOO_LOW        = 0x06070013, /* Data type does not match, length of service
                                                       parameter too low */
   CO_SDO_ABORT_BAD_SUBINDEX          = 0x06090011, /* Sub-index does not exist. */
   CO_SDO_ABORT_VALUE                 = 0x06090030, /* Invalid value for parameter (download only). */
   CO_SDO_ABORT_VALUE_TOO_HIGH        = 0x06090031, /* Value of parameter written too high (download only). */
   CO_SDO_ABORT_VALUE_TOO_LOW         = 0x06090032, /* Value of parameter written too low (download only). */
   CO_SDO_ABORT_MAX_LT_MIN            = 0x06090036, /* Maximum value is less than minimum value. */
   CO_SDO_ABORT_OUT_OF_RESOURCE       = 0x060A0023, /* Resource not available: SDO connection */
   CO_SDO_ABORT_GENERAL               = 0x08000000, /* General error */
   CO_SDO_ABORT_WRITE                 = 0x08000020, /* Data cannot be transferred or stored to the
                                                       application. */
   CO_SDO_ABORT_WRITE_LOCAL_DENIED    = 0x08000021, /* Data cannot be transferred or stored to the
                                                       application because of local control. */
   CO_SDO_ABORT_WRITE_STATE_DENIED    = 0x08000022, /* Data cannot be transferred or stored to the
                                                       application because of the present device state. */
   CO_SDO_ABORT_BAD_OD                = 0x08000023, /* Object dictionary dynamic generation fails or no
                                                       object dictionary is present (e.g. object dictionary
                                                       is generated from file and generation fails
                                                       because of an file error). */
   CO_SDO_ABORT_NO_DATA               = 0x08000024, /* No data available */
} co_sdo_abort_t;

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
void co_sdo_abort (co_net_t * net, uint16_t id, uint16_t index, uint8_t subindex, uint32_t code);

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
