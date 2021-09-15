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
 * @brief Core definitions
 */

#ifndef CO_MAIN_H
#define CO_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_api.h"
#include "osal.h"
#include "coal_can.h"
#include "options.h"
#include "osal_log.h"

#include <stdbool.h>

#define CO_BYTELENGTH(bitlength) (((bitlength) + 7) / 8)

#define CO_RTR_MASK   BIT (30)
#define CO_EXT_MASK   BIT (29)
#define CO_ID_MASK    0x1FFFFFFF
#define CO_EXTID_MASK (CO_EXT_MASK | CO_ID_MASK)

#define CO_COBID_INVALID BIT (31)
#define CO_NODE_GET(id)  ((id)&0x7F)

/* Pre-defined connection set (see CiA 301 chapter 7.3.3) */
#define CO_FUNCTION_NMT     (0 << 7)
#define CO_FUNCTION_SYNC    (1 << 7)
#define CO_FUNCTION_TIME    (2 << 7)
#define CO_FUNCTION_EMCY    (1 << 7)
#define CO_FUNCTION_PDO1_TX (3 << 7)
#define CO_FUNCTION_PDO1_RX (4 << 7)
#define CO_FUNCTION_PDO2_TX (5 << 7)
#define CO_FUNCTION_PDO2_RX (6 << 7)
#define CO_FUNCTION_PDO3_TX (7 << 7)
#define CO_FUNCTION_PDO3_RX (8 << 7)
#define CO_FUNCTION_PDO4_TX (9 << 7)
#define CO_FUNCTION_PDO4_RX (10 << 7)
#define CO_FUNCTION_SDO_TX  (11 << 7)
#define CO_FUNCTION_SDO_RX  (12 << 7)
#define CO_FUNCTION_NMT_ERR (14 << 7)
#define CO_FUNCTION_LSS     (15 << 7)
#define CO_FUNCTION_MASK    (15 << 7)

/**
 * Process data object (PDO)
 */
typedef struct co_pdo
{
   uint16_t number;
   uint32_t cobid;
   uint8_t transmission_type;
   uint8_t sync_start;
   uint8_t sync_counter;
   uint16_t inhibit_time;
   uint16_t event_timer;
   uint32_t timestamp;
   uint64_t frame;
   size_t bitlength;
   uint8_t number_of_mappings;
   struct
   {
      bool queued : 1;
      bool sync_wait : 1;
   };
   uint32_t mappings[MAX_PDO_ENTRIES];
   const co_obj_t * objs[MAX_PDO_ENTRIES];
   const co_entry_t * entries[MAX_PDO_ENTRIES];
} co_pdo_t;

typedef enum co_job_type
{
   CO_JOB_NONE,
   CO_JOB_PERIODIC,
   CO_JOB_RX,
   CO_JOB_PDO_EVENT,
   CO_JOB_PDO_OBJ_EVENT,
   CO_JOB_SDO_READ,
   CO_JOB_SDO_WRITE,
   CO_JOB_SDO_UPLOAD,
   CO_JOB_SDO_DOWNLOAD,
   CO_JOB_EMCY_TX,
   CO_JOB_ERROR_SET,
   CO_JOB_ERROR_CLEAR,
   CO_JOB_ERROR_GET,
   CO_JOB_EXIT,
} co_job_type_t;

/** Parameters for SDO job */
typedef struct co_sdo_job
{
   uint8_t node;
   uint16_t index;
   uint8_t subindex;
   uint8_t * data;
   uint64_t value;
   size_t remain;
   size_t total;
   struct
   {
      bool toggle : 1;
      bool cached : 1;
   };
} co_sdo_job_t;

/** Parameters for emergency job */
typedef struct co_emcy_job
{
   uint16_t code;
   uint16_t info;
   uint8_t * msef;
   uint8_t value;
} co_emcy_job_t;

/** Parameters for PDO job */
typedef struct co_pdo_job
{
   uint16_t index;
   uint8_t subindex;
} co_pdo_job_t;

/** Generic job */
typedef struct co_job
{
   co_job_type_t type;
   union
   {
      co_sdo_job_t sdo;
      co_emcy_job_t emcy;
      co_pdo_job_t pdo;
   };
   uint32_t timestamp;
   struct co_client * client;
   void (*callback) (struct co_job * job);
   int result;
} co_job_t;

/** Client state */
struct co_client
{
   os_sem_t * sem;
   co_job_t job;
   co_net_t * net;
};

/** Heartbeat consumer state */
typedef struct co_heartbeat
{
   uint8_t node;
   bool is_alive;
   uint16_t time;
   uint32_t timestamp;
} co_heartbeat_t;

/** Node guarding state */
typedef struct co_node_guard
{
   bool is_alive;
   uint16_t guard_time;
   uint8_t life_time_factor;
   uint8_t toggle;
   uint32_t timestamp;
} co_node_guard_t;

/** LSS states */
typedef enum lss_state
{
   LSS_STATE_WAITING = 0,
   LSS_STATE_CONFIG  = 1,
} lss_state_t;

/** LSS state */
typedef struct lss
{
   lss_state_t state;
   int bitrate;
   uint8_t node;
   const co_obj_t * identity;
   uint8_t match;
} lss_t;

/** SYNC producer state */
typedef struct co_sync
{
   uint32_t cobid;
   uint8_t counter;
   uint8_t overflow;
   uint32_t period;
   uint32_t timestamp;
} co_sync_t;

/** EMCY state */
typedef struct co_emcy
{
   uint32_t cobid;                   /**< EMCY COB ID */
   uint32_t timestamp;               /**< Timestamp of last EMCY */
   uint32_t bus_off_timestamp;       /**< Timestamp of bus-off event */
   uint16_t inhibit;                 /**< Inhibit time [100 us] */
   uint8_t error;                    /**< Error register */
   os_channel_state_t state;         /**< CAN state */
   bool node_guard_error;            /**< Node guard error */
   bool heartbeat_error;             /**< Heartbeat error */
   uint32_t cobids[MAX_EMCY_COBIDS]; /**< EMCY consumer object */
} co_emcy_t;

/** CANopen network state */
struct co_net
{
   os_channel_t * channel;      /**< CAN channel */
   int bitrate;                 /**< CAN bitrate (bits per second) */
   os_mbox_t * mbox;            /**< Mailbox for job submission */
   co_job_type_t job_periodic;  /**< Static message for periodic job */
   co_job_type_t job_rx;        /**< Static message for rx job */
   co_job_t job_sdo_server;     /**< Current SDO server job */
   co_job_t * job_client;       /**< Pointer to current client job */
   uint32_t nodes[4];           /**< Discovered nodes. 128-bit bitmap */
   uint8_t node;                /**< Node ID for this node */
   co_emcy_t emcy;              /**< EMCY state */
   co_sync_t sync;              /**< SYNC state */
   co_state_t state;            /**< NMT state */
   uint32_t hb_timestamp;       /**< Heartbeat producer timestamp */
   uint32_t hb_time;            /**< Heartbeat producer time */
   uint32_t sync_timestamp;     /**< Timestamp of last SYNC */
   uint32_t sync_window;        /**< Synchronous window length */
   uint32_t restart_ms;         /**< Delay before attempting to recover from bus-off */
   co_pdo_t pdo_tx[MAX_TX_PDO]; /**< TPDOs */
   co_pdo_t pdo_rx[MAX_RX_PDO]; /**< RPDOs */
   co_node_guard_t node_guard;  /**< Node guarding state */
   co_heartbeat_t heartbeat[MAX_HEARTBEATS]; /**< Heartbeat consumer state */
   uint8_t number_of_errors;                 /**< Number of active errors */
   uint32_t errors[MAX_ERRORS];              /**< List of active errors */
   uint8_t error_behavior;                   /**< Error behavior object */
   uint32_t config_date;                     /**< Configuration date */
   uint32_t config_time;                     /**< Configuration time */
   uint8_t config_dirty;                     /**< Configuration has changed */
   lss_t lss;                                /**< LSS state */
   const co_obj_t * od;                      /**< Object dictionary */
   const co_default_t * defaults;            /**< Dictionary default values */
   void * cb_arg;                            /**< Callback opaque argument */
   uint32_t mbox_overrun; /**< Mailbox overruns (for debugging) */

   /** Reset callback */
   void (*cb_reset) (co_net_t * net);

   /** NMT callback */
   void (*cb_nmt) (co_net_t * net, co_state_t state);

   /** SYNC callback */
   void (*cb_sync) (co_net_t * net);

   /** EMCY callback */
   bool (*cb_emcy) (
      co_net_t * net,
      uint8_t node,
      uint16_t code,
      uint8_t reg,
      uint8_t msef[5]);

   /** Notify callback */
   void (*cb_notify) (co_net_t * net, uint16_t index, uint8_t subindex);

   /** Function to open dictionary store */
   void * (*open) (co_store_t store, co_mode_t mode);

   /** Function to read from dictionary store */
   int (*read) (void * arg, void * data, size_t size);

   /** Function to write to dictionary store */
   int (*write) (void * arg, const void * data, size_t size);

   /** Function to close dictionary store */
   int (*close) (void * arg);
};

#ifdef __cplusplus
}
#endif

#endif /* CO_MAIN_H */
