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

/** Network handle
 *
 * Represents the entire CANopen network stack.
 * Its member fields are private.
 */
typedef struct co_net co_net_t;

/** Client handle
 *
 * Used for accessing the stack from a thread.
 * The same client handle should not be shared between threads.
 * Its member fields are private.
 */
typedef struct co_client co_client_t;

/* Status returned from function */
#define CO_STATUS_OK           0       /**< Function was successful */
#define CO_STATUS_ERROR       -1       /**< An error was detected */
#define CO_STATUS_SDO_TOGGLE  -2       /**< Not used */
#define CO_STATUS_SDO_TIMEOUT -3       /**< Not used */
#define CO_STATUS_SDO_UNKNOWN -4       /**< Not used */

/* Error register bits. See CiA 301 7.5.2.2  */
#define CO_ERR_GENERIC       (1U << 0) /**< Generic error */
#define CO_ERR_CURRENT       (1U << 1) /**< Current level error */
#define CO_ERR_VOLTAGE       (1U << 2) /**< Voltage level error */
#define CO_ERR_TEMPERATURE   (1U << 3) /**< Temperature error */
#define CO_ERR_COMMUNICATION (1U << 4) /**< Communication error */
#define CO_ERR_DEVICE        (1U << 5) /**< Device profile specific error */
#define CO_ERR_MANUFACTURER  (1U << 7) /**< Manufacturer-specific error */

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
 * machine. State values as used by e.g. the heartbeat protocol are
 * not used here.
 */
typedef enum co_state
{
   STATE_OFF = 0,                   /**< Power-on or hardware reset */
   STATE_INIT_PWRON,                /**< Initialisation - Initialising */
   STATE_INIT_APP,                  /**< Initialisation - Reset application */
   STATE_INIT_COMM,                 /**< Initialisation - Reset communication */
   STATE_INIT = STATE_INIT_COMM,    /**< Compatibility alias for NMT callbacks */
   STATE_PREOP,                     /**< Pre-operational */
   STATE_OP,                        /**< Operational */
   STATE_STOP,                      /**< Stopped */
   STATE_LAST                       /**< Sentinel. Not an actual state */
} co_state_t;

/** NMT commands */
typedef enum co_nmt_cmd
{
   CO_NMT_OPERATIONAL         = 0x01, /**< Start remote node */
   CO_NMT_STOPPED             = 0x02, /**< Stop remote node */
   CO_NMT_PRE_OPERATIONAL     = 0x80, /**< Enter pre-operational */
   CO_NMT_RESET_NODE          = 0x81, /**< Reset node */
   CO_NMT_RESET_COMMUNICATION = 0x82, /**< Reset communication */
} co_nmt_cmd_t;

/** Dictionary object types. See CiA 301 7.4.3 */
typedef enum co_otype
{
   OTYPE_NULL      = 0x00,          /**< An object with no data fields */
   OTYPE_DOMAIN    = 0x02,          /**< Large variable amount of data */
   OTYPE_DEFTYPE   = 0x05,          /**< Type definition */
   OTYPE_DEFSTRUCT = 0x06,          /**< Defines a new record type */
   OTYPE_VAR       = 0x07,          /**< Single value */
   OTYPE_ARRAY     = 0x08,          /**< A multiple data field object where
                                         each data field is a simple variable of
                                         the SAME basic data type */
   OTYPE_RECORD    = 0x09,          /**< A multiple data field object where the
                                         data fields may be any combination of
                                         simple variables */
} co_otype_t;

/** Dictionary datatypes. See CiA 301 7.4.7 */
typedef enum co_dtype
{
   DTYPE_BOOLEAN         = 0x0001,  /**< Boolean */
   DTYPE_INTEGER8        = 0x0002,  /**< Signed Integer (8 bits) */
   DTYPE_INTEGER16       = 0x0003,  /**< Signed Integer (16 bits) */
   DTYPE_INTEGER32       = 0x0004,  /**< Signed Integer (32 bits) */
   DTYPE_UNSIGNED8       = 0x0005,  /**< Unsigned Integer (8 bits) */
   DTYPE_UNSIGNED16      = 0x0006,  /**< Unsigned Integer (16 bits) */
   DTYPE_UNSIGNED32      = 0x0007,  /**< Unsigned Integer (32 bits) */
   DTYPE_REAL32          = 0x0008,  /**< Floating-Point (32 bites) */
   DTYPE_VISIBLE_STRING  = 0x0009,  /**< Visible String */
   DTYPE_OCTET_STRING    = 0x000A,  /**< Octet String */
   DTYPE_UNICODE_STRING  = 0x000B,  /**< Unicode String */
   DTYPE_TIME_OF_DAY     = 0x000C,  /**< Time of Day */
   DTYPE_TIME_DIFFERENCE = 0x000D,  /**< Time Difference */
   DTYPE_DOMAIN          = 0x000F,  /**< Domain */
   DTYPE_INTEGER24       = 0x0010,  /**< Signed Integer (24 bits) */
   DTYPE_REAL64          = 0x0011,  /**< Floating-Point (64 bites) */
   DTYPE_INTEGER40       = 0x0012,  /**< Signed Integer (40 bits) */
   DTYPE_INTEGER48       = 0x0013,  /**< Signed Integer (48 bits) */
   DTYPE_INTEGER56       = 0x0014,  /**< Signed Integer (56 bits) */
   DTYPE_INTEGER64       = 0x0015,  /**< Signed Integer (64 bits) */
   DTYPE_UNSIGNED24      = 0x0016,  /**< Unsigned Integer (24 bits) */
   DTYPE_UNSIGNED40      = 0x0018,  /**< Unsigned Integer (40 bits) */
   DTYPE_UNSIGNED48      = 0x0019,  /**< Unsigned Integer (48 bits) */
   DTYPE_UNSIGNED56      = 0x001A,  /**< Unsigned Integer (56 bits) */
   DTYPE_UNSIGNED64      = 0x001B,  /**< Unsigned Integer (64 bits) */
   DTYPE_PDO_COMM_PARAM  = 0x0020,  /**< PDO communication parameter */
   DTYPE_PDO_MAPPING     = 0x0021,  /**< PDO mapping parameter */
   DTYPE_SDO_PARAM       = 0x0022,  /**< SDO parameter */
   DTYPE_IDENTITY        = 0x0023,  /**< Identity */
} co_dtype_t;

/* Entry flags */
#define OD_READ      (1U << 0)      /**< Entry is readable */
#define OD_WRITE     (1U << 1)      /**< Entry is writable */
#define OD_TRANSIENT (1U << 2)      /**< Entry is transient (not persisted) */
#define OD_TPDO      (1U << 3)      /**< Entry is mappable as TPDO */
#define OD_RPDO      (1U << 4)      /**< Entry is mappable as RPDO */
#define OD_ARRAY     (1U << 5)      /**< Entry is an array */
#define OD_NOTIFY    (1U << 6)      /**< Entry notifies when updated */

/* Entry convenience flags */
#define OD_RO (OD_READ)             /**< Entry is read-only */
#define OD_WO (OD_WRITE)            /**< Entry is write-only */
#define OD_RW (OD_READ | OD_WRITE)  /**< Entry is read/write */

/** Access function event */
typedef enum od_event
{
   OD_EVENT_READ,                   /**< Read subindex */
   OD_EVENT_WRITE,                  /**< Write subindex */
   OD_EVENT_RESTORE,                /**< Restore default value */
} od_event_t;

struct co_obj;
struct co_entry;

/** Access function for object
 *
 * This function is called to programmatically handle access of an object
 * in the object dictionary.
 *
 * @code
 * uint32_t access_object (
 *   co_net_t * net,
 *   od_event_t event,
 *   const co_obj_t * obj,
 *   const co_entry_t * entry,
 *   uint8_t subindex,
 *   uint32_t * value)
 * {
 *    switch (event)
 *    {
 *    case OD_EVENT_READ:
 *       *value = foo[subindex];
 *       return 0;
 *    case OD_EVENT_WRITE:
 *       foo[subindex] = *value;
 *       return 0;
 *    case OD_EVENT_RESTORE:
 *       memset (foo, 0, sizeof (foo));
 *       return 0;
 *    }
 * }
 * @endcode
 *
 * Note that the stack implements access functions for many objects.
 * See the header file co_obj.h.
 *
 * @param net           network handle
 * @param event         access event (read, write, restore)
 * @param obj           object descriptor for index
 * @param entry         entry descriptor for subindex or range of subindexes
 * @param subindex      subindex. Range: 0 - obj->max_subindex
 * @param value         value to set or get
 *
 * @return 0 on success, error code otherwise (see @a co_sdo_abort_t)
 */
typedef uint32_t (*co_access_fn) (
   struct co_net * net,
   od_event_t event,
   const struct co_obj * obj,
   const struct co_entry * entry,
   uint8_t subindex,
   uint32_t * value
);

/** Entry descriptor
 *
 * Describes a subindex for an object in the object dictionary,
 * or a series of such subindexes as an array (if CO_ARRAY is set).
 *
 * The entry descriptor itself is never modified, but its associated data
 * might be. Data is associated with the entry in one of three ways:
 *  1. The @a access function in the object descriptor. May modify the data.
 *  2. The @a data pointer in the entry descriptor. The pointer is never
 *     modified, but the pointed data might be if the OD_WRITE flag is set.
 *  3. The @a value stored in the entry descriptor. Is never modified.
 * The stack resolves what data to use in the following order:
 *  1. If the entry's object @a access function is not NULL, then that is used.
 *  2. Otherwise, if the entry's a data pointer is not NULL, then that is used.
 *  3. Otherwise, the constant @a value is used.
 * As the two pointers @a access and @ data are both constant, the way to
 * resolve what data to use is also constant.
 *
 * Example for an object with a single subindex. The @a value field is used
 * to store a constant value as both the fields @a access and @a data are NULL:
 * @code
 * // Device Type at index 1000h, subindex 0
 * static const co_entry_t OD1000[] =
 * {
 *    {
 *       .subindex      = 0x00,
 *       .flags         = OD_RO,
 *       .datatype      = DTYPE_UNSIGNED32,
 *       .bitlength     = 32,
 *       .value         = 0x12345678, // 3. Constant value associated with entry
 *       .data          = NULL,       // 2. No data pointer
 *    },
 * };
 * static const co_obj_t od[] =
 * {
 *    {
 *       .index         = 0x1000,
 *       .objtype       = OTYPE_VAR,
 *       .max_subindex  = 0,
 *       .entries       = OD1000,
 *       .access        = NULL,       // 1. No access function
 *    },
 *    ...,
 *    {0},
 * };
 * @endcode
 *
 * Note that the stack implements entry descriptors for many objects.
 * See the header file co_obj.h.
 */
typedef struct co_entry
{
   uint8_t subindex;    /**< subindex */
   uint8_t flags;       /**< subindex flags */
   co_dtype_t datatype; /**< datatype */
   size_t bitlength;    /**< bitlength */
   uint32_t value;      /**< value. Not used if object @a access function or
                             @a data pointer are set */
   void * data;         /**< pointer to value. Not used if NULL or if
                             object @a access function is set */
} co_entry_t;

/** Object descriptor
 *
 * Describes an index in the object dictionary.
 *
 * Example for an object of type RECORD:
 * @code
 * // Identity object (1018h)
 * static const co_entry_t OD1018[] =
 * {
 *    {0x00, OD_RO, DTYPE_UNSIGNED8,   8, 4, NULL}, // Highest sub-index
 *    {0x01, OD_RO, DTYPE_UNSIGNED32, 32, 1, NULL}, // Vendor-ID
 *    {0x02, OD_RO, DTYPE_UNSIGNED32, 32, 2, NULL}, // Product code
 *    {0x03, OD_RO, DTYPE_UNSIGNED32, 32, 3, NULL}, // Revision number
 *    {0x04, OD_RO, DTYPE_UNSIGNED32, 32, 4, NULL}, // Serial number
 * };
 * static const co_obj_t od[] =
 * {
 *    ...,
 *    {
 *       .index         = 0x1018,
 *       .objtype       = OTYPE_RECORD,
 *       .max_subindex  = 4,
 *       .entries       = OD1018,
 *       .access        = NULL,
 *    },
 *    ...,
 *    {0},
 * };
 * @endcode
 */
typedef struct co_obj
{
   uint16_t index;             /**< index */
   co_otype_t objtype;         /**< type of object */
   uint8_t max_subindex;       /**< max subindex of object */
   const co_entry_t * entries; /**< list of entries in object */
   co_access_fn access;        /**< access function for object if not NULL */
} co_obj_t;

/** Default value for a subindex in the object dictionary
 *
 * The default value for a subindex is set whenever a node reset event or
 * a communication reset event occurs. The subindex needs to be writable.
 *
 * Example:
 * @code
 * // RPDO Communication Parameter 1
 * static const co_obj_t od[] =
 * {
 *    ...,
 *    {
 *       .index         = 0x1400,
 *       .objtype       = OTYPE_RECORD,
 *       .max_subindex  = 5,
 *       .entries       = OD1400,       // Implemented by the stack
 *       .access        = co_od1400_fn, // Implemented by the stack
 *    },
 *    ...,
 *    {0},
 * };
 * static const co_default_t od_defaults[] =
 * {
 *    ...,
 *    {
 *       .index      = 0x1400,
 *       .subindex   = 1,
 *       .value      = 0x200 + DEFAULT_NODE_ID,
 *    },
 *    ...,
 *    {0},
 * }
 * @endcode
 */
typedef struct co_default
{
   uint16_t index;   /**< index of object */
   uint8_t subindex; /**< subindex */
   uint64_t value;   /**< default value to set */
} co_default_t;

/** Parameter stores */
typedef enum co_store
{
   CO_STORE_COMM, /**< Communication objects */
   CO_STORE_APP,  /**< Application objects */
   CO_STORE_MFG,  /**< Manufacturer objects */
   CO_STORE_LSS,  /**< LSS data */
   CO_STORE_LAST, /**< Sentinel. Not an actual store */
} co_store_t;

/** Parameter store open modes */
typedef enum co_mode
{
   CO_MODE_READ,  /**< Open for reading */
   CO_MODE_WRITE, /**< Open for writing */
} co_mode_t;

/** Reset callback
 *
 * Called when a node reset has been ordered by NMT manager.
 * Note that this callback in not called for communication resets.
 *
 * @param net           network handle
 */
typedef void (*co_reset_fn) (co_net_t * net);

/** NMT callback
 *
 * Called when the NMT state has changed.
 *
 * @param net           network handle
 * @param state         new state
 */
typedef void (*co_nmt_fn) (co_net_t * net, co_state_t state);

/** SYNC callback
 *
 * Called when a SYNC message has been received or sent.
 *
 * @param net           network handle
 */
typedef void (*co_sync_fn) (co_net_t * net);

/** EMCY callback
 *
 * Called when
 * - CANopen stack has detected an error
 * - CANopen stack has determined that a previous error is no longer active
 * - An EMCY message from another node has been received and consumed
 *
 * Return true to enable error behavior. Error behavior could e.g be to switch
 * NMT state and is configurable in object dictionary object 0x1029.
 *
 * @code
 * bool emcy (co_net_t * net, uint8_t node, uint16_t code, uint8_t reg,
 *            uint8_t msef[5])
 * {
 *    if (code == 0x0000)
 *       printf ("Error was reset\n");
 *    else
 *       printf ("New error. Code: 0x%04x, Reg: 0x%02x\n");
 *
 *    if (node == co_node_id_get (net))
 *    {
 *       printf ("An EMCY event was triggered locally\n");
 *       return true;
 *    }
 *    else
 *    {
 *       printf ("An EMCY message was received from %u\n", node);
 *       return false;
 *    }
 * }
 * @endcode
 *
 * @param net           network handle
 * @param node          node ID of EMCY producer
 * @param code          error code
 * @param reg           error register
 * @param msef          manufacturer-specific error code. May be NULL
 *
 * @return true to enable error behavior, false otherwise.
 *         Not applicable for EMCY messages received from other nodes.
 */
typedef bool (*co_emcy_fn) (
   co_net_t * net,
   uint8_t node,
   uint16_t code,
   uint8_t reg,
   uint8_t msef[5]);

/** Notify callback
 *
 * Called when an entry in the object dictionary with the OD_NOTIFY flag set
 * has been written.
 *
 * @param net           network handle
 * @param index         index
 * @param subindex      subindex
 */
typedef void (*co_notify_fn) (co_net_t * net, uint16_t index, uint8_t subindex);

/** Heartbeat node state change callback
 *
 * Called when
 * - A Heartbeat message is received with new indicated state
 * - No heartbeat message was received from a node before timeout
 *
 * @param net           network handle
 * @param node          node ID of heartbeat producer
 * @param old_state     old state
 * @param new_state     new state
 */
typedef void (*co_heartbeat_state_fn) (
   co_net_t * net,
   uint8_t node,
   uint8_t old_state,
   uint8_t new_state);

/** Function to open parameter store
 *
 * @param store         parameter store
 * @param mode          mode
 *
 * @return file handle on success, NULL otherwise
 */
typedef void * (*co_open_fn) (co_store_t store, co_mode_t mode);

/** Function to read from parameter store
 *
 * @param file          file handle
 * @param data          data
 * @param size          size (in bytes)
 *
 * @return 0 on success, negative error code otherwise
 */
typedef int (*co_read_fn) (void * file, void * data, size_t size);

/** Function to write to parameter store
 *
 * @param file          file handle
 * @param data          data
 * @param size          size (in bytes)
 *
 * @return 0 on success, negative error code otherwise
 */
typedef int (*co_write_fn) (void * file, const void * data, size_t size);

/** Function to close parameter store
 *
 * @param file          file handle
 *
 * @return 0 on success, negative error code otherwise
 */
typedef int (*co_close_fn) (void * file);

/** CANopen stack configuration */
typedef struct co_cfg
{
   uint8_t node;        /**< Initial node ID */
   int bitrate;         /**< Initial bitrate (bits per second) */
   uint32_t restart_ms; /**< Bus-off recovery delay, zero to disable */
   const co_obj_t * od; /**< Application dictionary as 0-terminated array */
   const co_default_t * defaults; /**< Dictionary default values as
                                       0-terminated array */

   void * cb_arg;       /**< Callback opaque argument */
   co_reset_fn cb_reset;/**< Reset callback */
   co_nmt_fn cb_nmt;    /**< NMT callback */
   co_sync_fn cb_sync;  /**< SYNC callback */
   co_emcy_fn cb_emcy;  /**< EMCY callback */
   co_notify_fn cb_notify; /**< Notify callback */
   co_heartbeat_state_fn cb_heartbeat_state; /**< Heartbeat node state change
                                                  callback */
   co_open_fn open;     /**< Function to open parameter store */
   co_read_fn read;     /**< Function to read from parameter store */
   co_write_fn write;   /**< Function to write to parameter store */
   co_close_fn close;   /**< Function to close parameter store */
} co_cfg_t;


/**
 * Initialise CANopen stack
 *
 * This function initialises the stack and starts the server thread.
 *
 * @param canif         name of can channel interface
 * @param cfg           stack configuration
 *
 * @return network handle on success, NULL otherwise
 */
CO_EXPORT co_net_t * co_init (const char * canif, const co_cfg_t * cfg);

/**
 * Initialise client
 *
 * This function initialises a client. The client is used to submit
 * jobs to the stack server thread.
 *
 * Multiple client handles may be used, but each handle should only be used
 * by a single thread.
 *
 * @param net           network handle
 *
 * @return client handle on success, NULL otherwise
 */
CO_EXPORT co_client_t * co_client_init (co_net_t * net);

/**
 * Get next active node ID.
 *
 * This function returns the next active node ID, i.e. a node that has
 * sent an NMT boot-up message on the network and has not failed its
 * error control protocol.
 *
 * This function can be used to iterate over active nodes.
 *
 * @code
 * node = co_node_next (client, 0);
 * if (node == 0)
 * {
 *    printf ("No nodes found\n");
 * }
 * else
 * {
 *    while (node > 0)
 *    {
 *       printf ("Found node %d\n", node);
 *       node = co_node_next (client, node + 1);
 *    }
 * }
 * @endcode
 *
 * @param client        client handle
 * @param node          current node id
 *
 * @return next active node id if found, 0 otherwise
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
CO_EXPORT int co_pdo_obj_event (
   co_client_t * client,
   uint16_t index,
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
 * @return Same as @a size on success, CO_STATUS error code otherwise
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
 * @return Same as @a size on success, CO_STATUS error code otherwise
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
 * manufacturer-specific error code can be included.
 *
 * Calling this function adds an error to the error history object
 * (1003h). It also signals an error to the NMT state-machine which
 * may change state according to the setting of the error behavior
 * object (1029h) and the return value of the EMCY callback.
 *
 * The application will be notified via the EMCY callback. The node id
 * will be the active node id for this node.
 *
 * @param client        client handle
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
