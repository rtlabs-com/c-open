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
 * @brief Public API for CANopen stack
 */

#ifndef CO_API_H
#define CO_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "co_export.h"
#include "co_options.h"

/** Network handle */
typedef struct co_net co_net_t;

/** Client handle */
typedef struct co_client co_client_t;

#define CO_STATUS_OK          0
#define CO_STATUS_ERROR       -1
#define CO_STATUS_SDO_TOGGLE  -2
#define CO_STATUS_SDO_TIMEOUT -3
#define CO_STATUS_SDO_UNKNOWN -4

/* Error register bits */
#define CO_ERR_GENERIC       (1U << 0)
#define CO_ERR_CURRENT       (1U << 1)
#define CO_ERR_VOLTAGE       (1U << 2)
#define CO_ERR_TEMPERATURE   (1U << 3)
#define CO_ERR_COMMUNICATION (1U << 4)
#define CO_ERR_DEVICE        (1U << 5)
#define CO_ERR_MANUFACTURER  (1U << 6)

/** Abort error codes. See CiA 301 7.2.4 */
typedef enum co_sdo_abort
{
   // clang-format off
   /** Toggle bit not alternated. */
   CO_SDO_ABORT_TOGGLE                = 0x05030000,
   /** SDO protocol timed out. */
   CO_SDO_ABORT_TIMEOUT               = 0x05040000,
   /** Client/server command specifier not valid or unknown. */
   CO_SDO_ABORT_UNKNOWN               = 0x05040001,
   /** Invalid block size (block mode only). */
   CO_SDO_ABORT_INVALID_BLOCK_SIZE    = 0x05040002,
   /** Invalid sequence number (block mode only). */
   CO_SDO_ABORT_INVALID_SEQ_NO        = 0x05040003,
   /** CRC error (block mode only). */
   CO_SDO_ABORT_CRC_ERROR             = 0x05040004,
   /** Out of memory. */
   CO_SDO_ABORT_OUT_OF_MEMORY         = 0x05040005,
   /** Unsupported access to an object. */
   CO_SDO_ABORT_ACCESS                = 0x06010000,
   /** Attempt to read a write only object. */
   CO_SDO_ABORT_ACCESS_WO             = 0x06010001,
   /** Attempt to write a read only object. */
   CO_SDO_ABORT_ACCESS_RO             = 0x06010002,
   /** Object does not exist in the object dictionary. */
   CO_SDO_ABORT_BAD_INDEX             = 0x06020000,
   /** Object cannot be mapped to the PDO. */
   CO_SDO_ABORT_UNMAPPABLE            = 0x06040041,
   /** The number and length of the objects to be mapped would exceed
      PDO length. */
   CO_SDO_ABORT_PDO_LENGTH            = 0x06040042,
   /** General parameter incompatibility reason. */
   CO_SDO_ABORT_PARAM_INCOMPATIBLE    = 0x06040043,
   /** General internal incompatibility in the device. */
   CO_SDO_ABORT_INTERNAL_INCOMPATIBLE = 0x06040047,
   /** Access failed due to a hardware error. */
   CO_SDO_ABORT_HW_ERROR              = 0x06060000,
   /** Data type does not match, length of service parameter does not
      match */
   CO_SDO_ABORT_LENGTH                = 0x06070010,
   /** Data type does not match, length of service parameter too
      high */
   CO_SDO_ABORT_LENGTH_TOO_HIGH       = 0x06070012,
   /** Data type does not match, length of service parameter too
      low */
   CO_SDO_ABORT_LENGTH_TOO_LOW        = 0x06070013,
   /** Sub-index does not exist. */
   CO_SDO_ABORT_BAD_SUBINDEX          = 0x06090011,
   /** Invalid value for parameter (download only). */
   CO_SDO_ABORT_VALUE                 = 0x06090030,
   /** Value of parameter written too high (download only). */
   CO_SDO_ABORT_VALUE_TOO_HIGH        = 0x06090031,
   /** Value of parameter written too low (download only). */
   CO_SDO_ABORT_VALUE_TOO_LOW         = 0x06090032,
   /** Maximum value is less than minimum value. */
   CO_SDO_ABORT_MAX_LT_MIN            = 0x06090036,
   /** Resource not available: SDO connection */
   CO_SDO_ABORT_OUT_OF_RESOURCE       = 0x060A0023,
   /** General error */
   CO_SDO_ABORT_GENERAL               = 0x08000000,
   /** Data cannot be transferred or stored to the application. */
   CO_SDO_ABORT_WRITE                 = 0x08000020,
   /** Data cannot be transferred or stored to the application
      because of local control. */
   CO_SDO_ABORT_WRITE_LOCAL_DENIED    = 0x08000021,
   /** Data cannot be transferred or stored to the application
      because of the present device state. */
   CO_SDO_ABORT_WRITE_STATE_DENIED    = 0x08000022,
   /** Object dictionary dynamic generation fails or no object
      dictionary is present (e.g. object dictionary is generated from
      file and generation fails because of an file error). */
   CO_SDO_ABORT_BAD_OD                = 0x08000023,
   /** No data available */
   CO_SDO_ABORT_NO_DATA               = 0x08000024,
   // clang-format on
} co_sdo_abort_t;

/**
 * NMT states, see CiA 301 chapter 7.3.2
 *
 * Note that states are sequentially numbered to simplify the state
 * machine. State values as used by e.g. the heartbeart protocol are
 * not used here.
 */
typedef enum co_state
{
   STATE_OFF = 0,
   STATE_INIT_PWRON,
   STATE_INIT_APP,
   STATE_INIT_COMM,
   STATE_INIT = STATE_INIT_COMM, /* Compatibility alias for NMT callbacks */
   STATE_PREOP,
   STATE_OP,
   STATE_STOP,
   STATE_LAST
} co_state_t;

/** NMT commands */
typedef enum co_nmt_cmd
{
   CO_NMT_OPERATIONAL         = 0x01,
   CO_NMT_STOPPED             = 0x02,
   CO_NMT_PRE_OPERATIONAL     = 0x80,
   CO_NMT_RESET_NODE          = 0x81,
   CO_NMT_RESET_COMMUNICATION = 0x82,
} co_nmt_cmd_t;

/** Dictionary object types. See CiA 301 7.4.3 */
typedef enum co_otype
{
   OTYPE_NULL      = 0x00,
   OTYPE_DOMAIN    = 0x02,
   OTYPE_DEFTYPE   = 0x05,
   OTYPE_DEFSTRUCT = 0x06,
   OTYPE_VAR       = 0x07,
   OTYPE_ARRAY     = 0x08,
   OTYPE_RECORD    = 0x09,
} co_otype_t;

/** Dictionary datatypes. See CiA 301 7.4.7 */
typedef enum co_dtype
{
   DTYPE_BOOLEAN         = 0x0001,
   DTYPE_INTEGER8        = 0x0002,
   DTYPE_INTEGER16       = 0x0003,
   DTYPE_INTEGER32       = 0x0004,
   DTYPE_UNSIGNED8       = 0x0005,
   DTYPE_UNSIGNED16      = 0x0006,
   DTYPE_UNSIGNED32      = 0x0007,
   DTYPE_REAL32          = 0x0008,
   DTYPE_VISIBLE_STRING  = 0x0009,
   DTYPE_OCTET_STRING    = 0x000A,
   DTYPE_UNICODE_STRING  = 0x000B,
   DTYPE_TIME_OF_DAY     = 0x000C,
   DTYPE_TIME_DIFFERENCE = 0x000D,
   DTYPE_DOMAIN          = 0x000F,
   DTYPE_INTEGER24       = 0x0010,
   DTYPE_REAL64          = 0x0011,
   DTYPE_INTEGER40       = 0x0012,
   DTYPE_INTEGER48       = 0x0013,
   DTYPE_INTEGER56       = 0x0014,
   DTYPE_INTEGER64       = 0x0015,
   DTYPE_UNSIGNED24      = 0x0016,
   DTYPE_UNSIGNED40      = 0x0018,
   DTYPE_UNSIGNED48      = 0x0019,
   DTYPE_UNSIGNED56      = 0x001A,
   DTYPE_UNSIGNED64      = 0x001B,
   DTYPE_PDO_COMM_PARAM  = 0x0020,
   DTYPE_PDO_MAPPING     = 0x0021,
   DTYPE_SDO_PARAM       = 0x0022,
   DTYPE_IDENTITY        = 0x0023,
} co_dtype_t;

/* Entry flags */
#define OD_READ      (1U << 0) /**< Entry is readable */
#define OD_WRITE     (1U << 1) /**< Entry is writeable */
#define OD_TRANSIENT (1U << 2) /**< Entry is transient (not persisted) */
#define OD_TPDO      (1U << 3) /**< Entry is mappable as TPDO */
#define OD_RPDO      (1U << 4) /**< Entry is mappable as RPDO */
#define OD_ARRAY     (1U << 5) /**< Entry is an array */
#define OD_NOTIFY    (1U << 6) /**< Entry notifies when updated */

/* Entry convenience flags */
#define OD_RO (OD_READ)
#define OD_WO (OD_WRITE)
#define OD_RW (OD_READ | OD_WRITE)

/** Access function event */
typedef enum od_event
{
   OD_EVENT_READ,    /**< Read subindex */
   OD_EVENT_WRITE,   /**< Write subindex */
   OD_EVENT_RESTORE, /**< Restore default value */
} od_event_t;

struct co_obj;
struct co_entry;

/** Access function for object. This function is called to
    programmatically handle access of the object. */
typedef uint32_t (*co_access_fn) (
   struct co_net *,               /**< network handle */
   od_event_t event,              /**< access event */
   const struct co_obj * obj,     /**< object descriptor */
   const struct co_entry * entry, /**< entry descriptor */
   uint8_t subindex,              /**< subindex */
   uint32_t * value               /**< value to set or get */
);

/** Entry descriptor. Describes a subindex, or a series of subindexes
    as an array. */
typedef struct co_entry
{
   uint8_t subindex;    /**< subindex */
   uint8_t flags;       /**< subindex flags */
   co_dtype_t datatype; /**< datatype */
   size_t bitlength;    /**< bitlength */
   uint32_t value;      /**< current value if data is NULL */
   void * data;         /**< pointer to value */
} co_entry_t;

/** Object descriptor. Describes an index. */
typedef struct co_obj
{
   uint16_t index;             /**< index */
   co_otype_t objtype;         /**< type of object */
   uint8_t max_subindex;       /**< max subindex of object */
   const co_entry_t * entries; /**< list of entries in object */
   co_access_fn access;        /**< access function for object if not NULL */
} co_obj_t;

/** Default value for subindex */
typedef struct co_default
{
   uint16_t index;   /**< index of object */
   uint8_t subindex; /**< subindex */
   uint64_t value;   /**< default value to set */
} co_default_t;

/** Dictionary stores */
typedef enum co_store
{
   CO_STORE_COMM, /**< Communication objects */
   CO_STORE_APP,  /**< Application objects */
   CO_STORE_MFG,  /**< Manufacturer objects */
   CO_STORE_LSS,  /**< LSS data */
   CO_STORE_LAST,
} co_store_t;

/** Dictionary store open modes */
typedef enum co_mode
{
   CO_MODE_READ,  /**< Open for reading */
   CO_MODE_WRITE, /**< Open for writing */
} co_mode_t;

/** CANopen stack configuration */
typedef struct co_cfg
{
   uint8_t node;                  /**< Initial node ID */
   int bitrate;                   /**< Initial bitrate (bits per second) */
   uint32_t restart_ms;           /**< Bus-off recovery delay, zero to disable */
   const co_obj_t * od;           /**< Application dictionary */
   const co_default_t * defaults; /**< Dictionary default values */
   void * cb_arg;                 /**< Callback opaque argument */

   /** Reset callback */
   void (*cb_reset) (co_net_t * net);

   /** NMT callback */
   void (*cb_nmt) (co_net_t * net, co_state_t state);

   /** SYNC callback */
   void (*cb_sync) (co_net_t * net);

   /** EMCY callback, return true to enable error behavior */
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
} co_cfg_t;

/**
 * Initialise CANopen stack
 *
 * This function initialises the stack.
 *
 * @param canif         name of can channel interface
 * @param cfg           stack configuration
 *
 * @return network handle
 */
CO_EXPORT co_net_t * co_init (const char * canif, const co_cfg_t * cfg);

/**
 * Initialise client
 *
 * This function initialises a client. The client is used to submit
 * jobs to the stack.
 *
 * @param net           network handle
 *
 * @return client handle
 */
CO_EXPORT co_client_t * co_client_init (co_net_t * net);

/**
 * Get next active node ID.
 *
 * This function returns the next active node ID, i.e. a node that has
 * sent an NMT bootup message on the network. This function can be
 * used to iterate over active nodes.
 *
 * @code
 * node = co_node_next (net, 0);
 * if (node == 0)
 * {
 *    printf ("No nodes found\n");
 * }
 * else
 * {
 *    while (node > 0)
 *    {
 *       printf ("Found node %d\n", node);
 *       node = co_node_next (net, node + 1);
 *    }
 * }
 * @endcode
 *
 * @param client        client handle
 * @param node          current node
 *
 * @return next active node
 */
CO_EXPORT uint8_t co_node_next (co_client_t * client, uint8_t node);

/**
 * Get active node ID.
 *
 * This function returns this node's active node ID, i.e. the
 * default ID set to co_init(), unless overridden via LSS.
 *
 * @param net           network handle
 *
 * @return the active node id
 */
CO_EXPORT uint8_t co_node_id_get (co_net_t * net);

/**
 * Get callback argument.
 *
 * This function returns the callback opaque argument specified to
 * co_init(), for example to reach application specific context from
 * object access functions and other callbacks.
 *
 * @param net           network handle
 *
 * @return the opaque callback argument
 */
CO_EXPORT void * co_cb_arg_get (co_net_t * net);

/**
 * Send NMT command.
 *
 * This function sends an NMT command to the given node.
 *
 * @param client        client handle
 * @param cmd           NMT command
 * @param node          node ID, or 0 for broadcast
 */
CO_EXPORT void co_nmt (co_client_t * client, co_nmt_cmd_t cmd, uint8_t node);

/**
 * Send SYNC message.
 *
 * This function broadcasts a SYNC message. TODO: remove?
 *
 * @param client        client handle
 */
CO_EXPORT void co_sync (co_client_t * client);

/**
 * Trigger event-based PDOs
 *
 * This function triggers transmission of all event-based PDOs.
 *
 * @param client  client handle
 *
 * @return 0 on success
 */
CO_EXPORT int co_pdo_event (co_client_t * client);

/**
 * Triggers event-based PDOs containing a specific object
 *
 * This function triggers transmission of all event-based PDOs
 * that map the specified object.
 *
 * @param client    client handle
 * @param index     index
 * @param subindex  subindex
 *
 * @return 0 on success
 */
CO_EXPORT int co_pdo_obj_event (co_client_t * client, uint16_t index,
                                uint8_t subindex);

/**
 * Read dictionary object entry
 *
 * This function reads an object entry from a device.
 *
 * @param client        client handle
 * @param node          node ID
 * @param index         index
 * @param subindex      subindex
 * @param data          storage for result
 * @param size          number of bytes to read
 *
 * @return 0 on success, CO_STATUS error code otherwise
 */
CO_EXPORT int co_sdo_read (
   co_client_t * client,
   uint8_t node,
   uint16_t index,
   uint8_t subindex,
   void * data,
   size_t size);

/**
 * Write dictionary object entry
 *
 * This function writes an object entry in a device.
 *
 * @param client        client handle
 * @param node          node ID
 * @param index         index
 * @param subindex      subindex
 * @param data          data to write
 * @param size          number of bytes to write
 *
 * @return 0 on success, CO_STATUS error code otherwise
 */
CO_EXPORT int co_sdo_write (
   co_client_t * client,
   uint8_t node,
   uint16_t index,
   uint8_t subindex,
   const void * data,
   size_t size);

/**
 * Transmit emergency object (EMCY)
 *
 * This function transmits an emergency object. An optional
 * manufacturer-speficic error code can be included.
 *
 * Calling this function adds an error to the error history object
 * (1003h). It also signals an error to the NMT state-machine which
 * may change state according to the setting of the error behavior
 * object (1029h) and the return value of the EMCY callback.
 *
 * The application will be notified via the EMCY callback. The node id
 * will be the active node id for this node.
 *
 * @param net           network handle
 * @param code          error code, lower 16 bits of error number
 * @param info          additional info, upper 16 bits of error number
 * @param msef          manufacturer-specific error code or NULL
 *
 * @return 0 on success, CO_STATUS error code otherwise
 */
CO_EXPORT int co_emcy_issue (
   co_client_t * client,
   uint16_t code,
   uint16_t info,
   uint8_t msef[5]);

/**
 * Set error register
 *
 * This function sets bits in the CANopen error register. The
 * application can set all bits, but note that the stack internally
 * handles CO_ERR_GENERIC and CO_ERR_COMMUNICATION. CO_ERR_GENERIC is
 * set if any other bit is set, and cleared
 * otherwise. CO_ERR_COMMUNICATION is set if the stack detects a
 * communication problem.
 *
 * @param client        client handle
 * @param mask          error register bits to set (CO_ERR bitmask)
 *
 * @return 0 on success, CO_STATUS error code otherwise
 */
CO_EXPORT int co_error_set (co_client_t * client, uint8_t mask);

/**
 * Clear error register
 *
 * This function clears bits in the CANopen error register.
 *
 * @param client        client handle
 * @param mask          error register bits to clear (CO_ERR bitmask)
 *
 * @return 0 on success, CO_STATUS error code otherwise
 */
CO_EXPORT int co_error_clear (co_client_t * client, uint8_t mask);

/**
 * Get error register
 *
 * This function gets the CANopen error register value.
 *
 * @param client        client handle
 * @param error         error register value (CO_ERR bitmask)
 *
 * @return 0 on success, CO_STATUS error code otherwise
 */
CO_EXPORT int co_error_get (co_client_t * client, uint8_t * error);

#ifdef __cplusplus
}
#endif

#endif /* CO_API_H */
