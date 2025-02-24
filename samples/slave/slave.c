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

#include <stdio.h>
#include <string.h>

#include "co_api.h"
#include "co_obj.h"

#include "osal.h"

/* This file contains a simple slave. It serves as an example of stack
   usage and can also be used to run the conformance test using the
   Conformance Test Tool (CTT). */

#define DEFAULT_NODE_ID 1

static char mfr_device_name[] = "CANopen sample slave";
static char mfr_hw_version[]  = "1.0";
static char mfr_sw_version[]  = "0.1";

/* Contains 64-bit TPDO */
static uint64_t output;

/* Contains 16-bit RPDO */
static uint16_t input;

/* These are used for conformance test purposes */
static char os_command[20];
static char os_reply[20];
static uint8_t os_mode;

/* Entry descriptor for Device type (1000h) */
static const co_entry_t OD1000[] = {
   {0x00, OD_RO, DTYPE_UNSIGNED32, 32, 0x12345678, NULL},
};

/* Entry descriptor for Manufacturer device name (1008h) */
static const co_entry_t OD1008[] = {
   {0x00, OD_RO, DTYPE_VISIBLE_STRING, 8 * sizeof (mfr_device_name), 0, mfr_device_name},
};

/* Entry descriptor for Manufacturer hardware version (1009h) */
static const co_entry_t OD1009[] = {
   {0x00, OD_RO, DTYPE_VISIBLE_STRING, 8 * sizeof (mfr_hw_version), 0, mfr_hw_version},
};

/* Entry descriptor for Manufacturer software version (100Ah) */
static const co_entry_t OD100A[] = {
   {0x00, OD_RO, DTYPE_VISIBLE_STRING, 8 * sizeof (mfr_sw_version), 0, mfr_sw_version},
};

/* Entry descriptor for Identity object (1018h) */
static const co_entry_t OD1018[] = {
   {0x00, OD_RO, DTYPE_UNSIGNED8, 8, 4, NULL},
   {0x01, OD_RO, DTYPE_UNSIGNED32, 32, 1, NULL},
   {0x02, OD_RO, DTYPE_UNSIGNED32, 32, 2, NULL},
   {0x03, OD_RO, DTYPE_UNSIGNED32, 32, 3, NULL},
   {0x04, OD_RO, DTYPE_UNSIGNED32, 32, 4, NULL},
};

/* Entry descriptor for OS command (1023h) (for CTT) */
static const co_entry_t OD1023[] = {
   {0x00, OD_RO, DTYPE_UNSIGNED8, 8, 3, NULL},
   {0x01,
    OD_WO | OD_TRANSIENT,
    DTYPE_OCTET_STRING,
    8 * sizeof (os_command),
    0,
    os_command},
   {0x02, OD_RO, DTYPE_UNSIGNED8, 8, 0, NULL},
   {0x03, OD_RO, DTYPE_OCTET_STRING, 8 * sizeof (os_reply), 0, os_reply},
};

/* Entry descriptor for OS command mode (1024h) (for CTT) */
static const co_entry_t OD1024[] = {
   {0x00, OD_WO, DTYPE_UNSIGNED8, 8, 0, &os_mode},
};

/* Entry descriptor for sample 64-bit actuator (2000h) */
static const co_entry_t OD2000[] = {
   {0x00, OD_NOTIFY | OD_RW | OD_RPDO | OD_TRANSIENT, DTYPE_UNSIGNED64, 64, 0, &output},
};

/* Entry descriptor for sample 16-bit sensor (2001h) */
static const co_entry_t OD2001[] = {
   {0x00, OD_RO | OD_TPDO, DTYPE_UNSIGNED16, 16, 0, &input},
};

/* Object dictionary for the slave. Note that some objects are defined
   and handled by the stack. */
static const co_obj_t od[] = {
   // clang-format off
   {0x1000, OTYPE_VAR,    0,               OD1000, NULL},
   {0x1001, OTYPE_VAR,    0,               OD1001, co_od1001_fn},
   {0x1003, OTYPE_ARRAY,  MAX_ERRORS,      OD1003, co_od1003_fn},
   {0x1005, OTYPE_VAR,    0,               OD1005, co_od1005_fn},
   {0x1006, OTYPE_VAR,    0,               OD1006, co_od1006_fn},
   {0x1007, OTYPE_VAR,    0,               OD1007, co_od1007_fn},
   {0x1008, OTYPE_VAR,    0,               OD1008, NULL},
   {0x1009, OTYPE_VAR,    0,               OD1009, NULL},
   {0x100A, OTYPE_VAR,    0,               OD100A, NULL},
   {0x100C, OTYPE_VAR,    0,               OD100C, co_od100C_fn},
   {0x100D, OTYPE_VAR,    0,               OD100D, co_od100D_fn},
   {0x1010, OTYPE_ARRAY,  4,               OD1010, co_od1010_fn},
   {0x1011, OTYPE_ARRAY,  4,               OD1011, co_od1011_fn},
   {0x1014, OTYPE_VAR,    0,               OD1014, co_od1014_fn},
   {0x1015, OTYPE_VAR,    0,               OD1015, co_od1015_fn},
   {0x1016, OTYPE_ARRAY,  MAX_HEARTBEATS,  OD1016, co_od1016_fn},
   {0x1017, OTYPE_VAR,    0,               OD1017, co_od1017_fn},
   {0x1018, OTYPE_RECORD, 4,               OD1018, NULL},
   {0x1019, OTYPE_VAR,    0,               OD1019, co_od1019_fn},
   {0x1020, OTYPE_ARRAY,  2,               OD1020, co_od1020_fn},
   {0x1023, OTYPE_RECORD, 3,               OD1023, NULL},
   {0x1024, OTYPE_VAR,    0,               OD1024, NULL},
   {0x1028, OTYPE_ARRAY,  MAX_EMCY_COBIDS, OD1028, co_od1028_fn},
   {0x1029, OTYPE_ARRAY,  1,               OD1029, co_od1029_fn},
   {0x1400, OTYPE_RECORD, 5,               OD1400, co_od1400_fn},
   {0x1600, OTYPE_RECORD, MAX_PDO_ENTRIES, OD1600, co_od1600_fn},
   {0x1800, OTYPE_RECORD, 6,               OD1800, co_od1800_fn},
   {0x1A00, OTYPE_RECORD, MAX_PDO_ENTRIES, OD1A00, co_od1A00_fn},
   {0x2000, OTYPE_VAR,    0,               OD2000, NULL},
   {0x2001, OTYPE_VAR,    0,               OD2001, NULL},
   {0},
   // clang-format on
};

/* Default values to be set when the NMT state INIT is entered */
static const co_default_t od_defaults[] = {
   {0x1400, 1, 0x200 + DEFAULT_NODE_ID}, /* Setup 1 RPDO */
   {0x1600, 0, 0x01},                    /* One mapping */
   {0x1600, 1, 0x20000040},              /* Map 2000h to RPDO (64 bits) */
   {0x1800, 1, 0x180 + DEFAULT_NODE_ID}, /* Setup 1 TPDO */
   {0x1A00, 0, 0x01},                    /* One mapping */
   {0x1A00, 1, 0x20010010},              /* Map 2001h to TPDO (16 bits) */
   {0x2000, 0, 0xDEADCAFEFEEDBEEF},      /* Default value for output */
   {0x2001, 0, 0x55AA},                  /* Default value for input */
   {0}};

/* Functions to simulate persistent storage. This uses RAM, while a
   real application would use filesystem or NVM for storage. */

static uint8_t stores[CO_STORE_LAST][1024];
static struct fd
{
   uint8_t * p;
} fd;

static void * store_open (co_store_t store, co_mode_t mode)
{
   if (store >= CO_STORE_LAST)
      return NULL;

   fd.p = stores[store];
   return &fd;
}

static int store_read (void * arg, void * data, size_t size)
{
   struct fd * fd = arg;
   memcpy (data, fd->p, size);
   fd->p += size;
   return 0;
}

static int store_write (void * arg, const void * data, size_t size)
{
   struct fd * fd = arg;
   memcpy (fd->p, data, size);
   fd->p += size;
   return 0;
}

static int store_close (void * arg)
{
   return 0;
}

/* Called when NMT command Reset node is received */
static void cb_reset (co_net_t * net)
{
   /* Optionally reset hardware */
}

/* Called when SYNC is received */
static void cb_sync (co_net_t * net)
{
   input++;
}

/* Called when RPDO is received (if OD_NOTIFY is set) */
static void cb_notify (co_net_t * net, uint16_t index, uint8_t subindex)
{
}

int slave_init (const char * canif, int bitrate)
{
   co_cfg_t cfg = {
      .node      = DEFAULT_NODE_ID,
      .bitrate   = bitrate,
      .od        = od,
      .defaults  = od_defaults,
      .cb_reset  = cb_reset,
      .cb_sync   = cb_sync,
      .cb_notify = cb_notify,
      .open      = store_open,
      .read      = store_read,
      .write     = store_write,
      .close     = store_close,
   };

   /* Initialise and start stack */
   co_net_t * net = co_init (canif, &cfg);
   if (net == NULL)
   {
      printf ("Init failed\n");
      return -1;
   }

   /* Initialise client */
   co_client_t * client = co_client_init (net);
   if (client == NULL)
   {
      printf ("Client init failed\n");
      return -1;
   }

   /* Wait a while, then generate error and emergency */
   os_usleep (5 * 1000);
   co_error_set (client, CO_ERR_MANUFACTURER);
   co_emcy_issue (client, 0x1000, 0x1234, NULL);

   /* Clear the error */
   os_usleep (5 * 1000);
   co_error_clear (client, CO_ERR_MANUFACTURER);

   return 0;
}
