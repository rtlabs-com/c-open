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

#ifdef UNIT_TEST
#define os_channel_send mock_os_channel_send
#define os_channel_bus_off mock_os_channel_bus_off
#define os_channel_bus_on mock_os_channel_bus_on
#define os_channel_set_bitrate mock_os_channel_set_bitrate
#endif

#include "co_lss.h"
#include "co_nmt.h"
#include "co_od.h"
#include "co_util.h"

typedef enum lss_cs
{
   CS_SWITCH_GLOBAL                  = 0x04,
   CS_CONFIGURE_NODE_ID              = 0x11,
   CS_CONFIGURE_BIT_TIMING           = 0x13,
   CS_ACTIVATE_BIT_TIMING            = 0x15,
   CS_STORE_CONFIGURATION            = 0x17,
   CS_SWITCH_SELECTIVE_VENDOR_ID     = 0x40,
   CS_SWITCH_SELECTIVE_PRODUCT_NO    = 0x41,
   CS_SWITCH_SELECTIVE_REVISION_NO   = 0x42,
   CS_SWITCH_SELECTIVE_SERIAL_NO     = 0x43,
   CS_SWITCH_SELECTIVE_SUCCESS       = 0x44,
   CS_IDENTIFY_REMOTE_VENDOR_ID      = 0x46,
   CS_IDENTIFY_REMOTE_PRODUCT_NO     = 0x47,
   CS_IDENTIFY_REMOTE_REVISION_LOW   = 0x48,
   CS_IDENTIFY_REMOTE_REVISION_HIGH  = 0x49,
   CS_IDENTIFY_REMOTE_SERIAL_LOW     = 0x4A,
   CS_IDENTIFY_REMOTE_SERIAL_HIGH    = 0x4B,
   CS_IDENTIFY_NON_CONFIGURED_REMOTE = 0x4C,
   CS_IDENTIFY_SLAVE                 = 0x4F,
   CS_IDENTIFY_NON_CONFIGURED_SLAVE  = 0x50,
   CS_INQUIRE_IDENTITY_VENDOR_ID     = 0x5A,
   CS_INQUIRE_IDENTITY_PRODUCT_NO    = 0x5B,
   CS_INQUIRE_IDENTITY_REVISION_NO   = 0x5C,
   CS_INQUIRE_IDENTITY_SERIAL_NO     = 0x5D,
   CS_INQUIRE_IDENTITY_NODE_ID       = 0x5E,
} lss_cs_t;

typedef enum lss_bitrate
{
   LSS_BITRATE_1M       = 0,
   LSS_BITRATE_800K     = 1,
   LSS_BITRATE_500K     = 2,
   LSS_BITRATE_250K     = 3,
   LSS_BITRATE_125K     = 4,
   LSS_BITRATE_RESERVED = 5,
   LSS_BITRATE_50K      = 6,
   LSS_BITRATE_20K      = 7,
   LSS_BITRATE_10K      = 8,
   LSS_BITRATE_AUTO     = 9,     /* TODO */
} lss_bitrate_t;

typedef enum lss_match_type
{
   MATCH_VENDOR_ID     = 0,
   MATCH_PRODUCT_NO    = 1,
   MATCH_REVISION_LOW  = 2,
   MATCH_REVISION_HIGH = 3,
   MATCH_SERIAL_LOW    = 4,
   MATCH_SERIAL_HIGH   = 5,
} lss_match_type_t;

typedef enum lss_store_error
{
   SUCCESS,
   NOT_SUPPORTED,
   STORAGE_ERROR,
} lss_store_error_t;

#define LSS_MATCH_ALL (BIT (MATCH_SERIAL_HIGH + 1) - 1)

static int co_lss_bitrates[] =
{
   1000 * 1000,
    800 * 1000,
    500 * 1000,
    250 * 1000,
    125 * 1000,
            -1,
     50 * 1000,
     20 * 1000,
     10 * 1000,
             0
};

const char * co_lss_state_literals[] =
{
   "STATE_WAITING",
   "STATE_CONFIG",
};

static uint32_t co_lss_identity_get (co_net_t * net, uint8_t subindex)
{
   uint64_t value;
   const co_entry_t * entry;

   entry = co_entry_find (net, net->lss.identity, subindex);
   co_od_get_value (net, net->lss.identity, entry, subindex, &value);
   return value & UINT32_MAX;
}

static void co_lss_match (co_net_t * net, uint8_t type, uint32_t expected)
{
   uint8_t mask = BIT (type) - 1;
   int is_match;

   /* Check that all previous matches were hits */
   if ((net->lss.match & mask) != mask)
      return;

   switch(type)
   {
   case MATCH_VENDOR_ID:
      is_match = co_lss_identity_get (net, 1) == expected;
      break;
   case MATCH_PRODUCT_NO:
      is_match = co_lss_identity_get (net, 2) == expected;
      break;
   case MATCH_REVISION_LOW:
      is_match = co_lss_identity_get (net, 3) >= expected;
      break;
   case MATCH_REVISION_HIGH:
      is_match = co_lss_identity_get (net, 3) <= expected;
      break;
   case MATCH_SERIAL_LOW:
      is_match = co_lss_identity_get (net, 4) >= expected;
      break;
   case MATCH_SERIAL_HIGH:
      is_match = co_lss_identity_get (net, 4) <= expected;
      break;
   default:
      is_match = 0;
   }

   /* Set/clear type bits according to match result */
   if (is_match)
      net->lss.match |= BIT (type);
   else
      net->lss.match = 0;       /* clear all previous results */
}

static void co_lss_switch_global (co_net_t * net, uint8_t * msg)
{
   uint8_t mode = msg[1];
   lss_state_t previous = net->lss.state;

   if (mode > LSS_STATE_CONFIG)
      return;

   LOG_DEBUG (CO_LSS_LOG, "lss state = %s\n", co_lss_state_literals[mode]);
   net->lss.state = mode;

   if (previous == LSS_STATE_CONFIG && net->lss.state == LSS_STATE_WAITING)
   {
      /* Enter WAITING state */
      if (net->state == STATE_INIT)
      {
         if (net->lss.node != 0xFF)
         {
            /* Copy pending node-ID to active node-ID */
            net->node = net->lss.node;
            co_nmt_event (net, EVENT_INITDONE);
         }
      }
   }
}

static void co_lss_configure_node_id (co_net_t * net, uint8_t * _msg)
{
   uint8_t node = _msg[1];
   uint8_t msg[8] = { 0 };

   /* Ignore if not in config state */
   if (net->lss.state != LSS_STATE_CONFIG)
      return;

   if ((node > 0 && node < 0x80) || node == 0xFF)
   {
      LOG_INFO (CO_LSS_LOG, "lss pending node id %d\n", node);
      net->lss.node = node;
   }
   else
   {
      LOG_DEBUG (CO_LSS_LOG, "lss bad node id %d\n", node);
      msg[1] = 1;               /* out of range */
   }

   msg[0] = CS_CONFIGURE_NODE_ID;
   os_channel_send (net->channel, 0x7E4, msg, sizeof(msg));
}

static void co_lss_configure_bit_timing (co_net_t * net, uint8_t * _msg)
{
   uint8_t table = _msg[1];
   uint8_t ix = _msg[2];
   uint8_t msg[8] = { 0 };

   /* Ignore if not in config state */
   if (net->lss.state != LSS_STATE_CONFIG)
      return;

   if (table == 0 &&
       ix <= LSS_BITRATE_10K && /* TODO: auto */
       ix != LSS_BITRATE_RESERVED)
   {
      LOG_DEBUG (CO_LSS_LOG, "lss pending bitrate ix %d\n", ix);
      net->lss.bitrate = co_lss_bitrates[ix];
   }
   else
   {
      LOG_DEBUG (CO_LSS_LOG, "lss bad bitrate ix %d\n", ix);
      msg[1] = 1;               /* bitrate not supported */
   }

   msg[0] = CS_CONFIGURE_BIT_TIMING;
   os_channel_send (net->channel, 0x7E4, msg, sizeof(msg));
}

static void co_lss_activate_bit_timing (co_net_t * net, uint8_t * _msg)
{
   uint16_t delay = co_fetch_uint16 (&_msg[1]);

   /* Ignore if not in config state */
   if (net->lss.state != LSS_STATE_CONFIG)
      return;

   os_usleep (1000 * delay);

   net->bitrate = net->lss.bitrate;
   os_channel_bus_off (net->channel);
   os_channel_set_bitrate (net->channel, net->bitrate);
   os_channel_bus_on (net->channel);

   os_usleep (1000 * delay);
}

static void co_lss_store_configuration (co_net_t * net, uint8_t * _msg)
{
   uint8_t msg[8] = { 0 };
   void * arg;

   /* Ignore if not in config state */
   if (net->lss.state != LSS_STATE_CONFIG)
      return;

   msg[0] = CS_STORE_CONFIGURATION;
   msg[1] = STORAGE_ERROR;

   if (net->open == NULL || net->write == NULL || net->close == NULL)
   {
      msg[1] = NOT_SUPPORTED;
      goto error1;
   }

   arg = net->open (CO_STORE_LSS);
   if (arg == NULL)
      goto error1;

   /* Store node id */
   if (net->write (arg, &net->lss.node, sizeof(net->lss.node)) < 0)
      goto error2;

   /* Store bitrate */
   if (net->write (arg, &net->lss.bitrate, sizeof(net->lss.bitrate)) < 0)
      goto error2;

   /* Finalize write */
   if (net->close (arg) < 0)
      goto error1;

   msg[1] = SUCCESS;
   os_channel_send (net->channel, 0x7E4, msg, sizeof(msg));
   return;

 error2:
   net->close (arg);
 error1:
   os_channel_send (net->channel, 0x7E4, msg, sizeof(msg));
}

static void co_lss_switch_selective (co_net_t * net, uint8_t * msg)
{
   uint8_t cmd = msg[0];
   uint32_t expected = co_fetch_uint32 (&msg[1]);

   switch(cmd)
   {
   case CS_SWITCH_SELECTIVE_VENDOR_ID:
      co_lss_match (net, MATCH_VENDOR_ID, expected);
      break;
   case CS_SWITCH_SELECTIVE_PRODUCT_NO:
      co_lss_match (net, MATCH_PRODUCT_NO, expected);
      break;
   case CS_SWITCH_SELECTIVE_REVISION_NO:
      co_lss_match (net, MATCH_REVISION_LOW, expected);
      co_lss_match (net, MATCH_REVISION_HIGH, expected);
      break;
   case CS_SWITCH_SELECTIVE_SERIAL_NO:
      co_lss_match (net, MATCH_SERIAL_LOW, expected);
      co_lss_match (net, MATCH_SERIAL_HIGH, expected);
      break;
   }

   /* Enter config mode if all requests matched */
   if (net->lss.match == LSS_MATCH_ALL)
   {
      uint8_t msg[8] = { 0 };

      net->lss.match = 0;
      net->lss.state = LSS_STATE_CONFIG;

      /* Indicate success */
      LOG_DEBUG (CO_LSS_LOG, "lss state = %s\n",
              co_lss_state_literals[LSS_STATE_CONFIG]);
      co_put_uint8 (msg, CS_SWITCH_SELECTIVE_SUCCESS);
      os_channel_send (net->channel, 0x7E4, msg, sizeof(msg));
   }
}

static void co_lss_inquire_identity (co_net_t * net, uint8_t * _msg)
{
   uint8_t cmd = _msg[0];
   uint8_t request = cmd - CS_INQUIRE_IDENTITY_VENDOR_ID;
   uint64_t value;
   const co_entry_t * entry;
   uint8_t msg[8] = { 0 };
   uint8_t * p;

   /* Ignore if not in config state */
   if (net->lss.state != LSS_STATE_CONFIG)
      return;

   /* Get value for this request */
   if (cmd == CS_INQUIRE_IDENTITY_NODE_ID)
   {
      value = net->node;
   }
   else
   {
      entry = co_entry_find (net, net->lss.identity, request + 1);
      co_od_get_value (net, net->lss.identity, entry, request + 1, &value);
   }

   /* Send response */
   p = co_put_uint8 (msg, cmd);
   co_put_uint32 (p, value & UINT32_MAX);
   os_channel_send (net->channel, 0x7E4, msg, sizeof(msg));
}

static void co_lss_identify_non_configured_remote (co_net_t * net)
{
   uint8_t msg[8] = { 0 };

   /* Ignore if not in config state */
   if (net->lss.state != LSS_STATE_CONFIG)
      return;

   if (net->state != STATE_INIT)
      return;

   /* TODO: check node-id state */

   /* Send response */
   co_put_uint8 (msg, CS_IDENTIFY_NON_CONFIGURED_SLAVE);
   os_channel_send (net->channel, 0x7E4, msg, sizeof(msg));
}

static void co_lss_identify_remote (co_net_t * net, uint8_t * msg)
{
   uint8_t cmd = msg[0];
   uint32_t expected = co_fetch_uint32 (&msg[1]);

   switch(cmd)
   {
   case CS_IDENTIFY_REMOTE_VENDOR_ID:
      co_lss_match (net, MATCH_VENDOR_ID, expected);
      break;
   case CS_IDENTIFY_REMOTE_PRODUCT_NO:
      co_lss_match (net, MATCH_PRODUCT_NO, expected);
      break;
   case CS_IDENTIFY_REMOTE_REVISION_LOW:
      co_lss_match (net, MATCH_REVISION_LOW, expected);
      break;
   case CS_IDENTIFY_REMOTE_REVISION_HIGH:
      co_lss_match (net, MATCH_REVISION_HIGH, expected);
      break;
   case CS_IDENTIFY_REMOTE_SERIAL_LOW:
      co_lss_match (net, MATCH_SERIAL_LOW, expected);
      break;
   case CS_IDENTIFY_REMOTE_SERIAL_HIGH:
      co_lss_match (net, MATCH_SERIAL_HIGH, expected);
      break;
   }

   /* Enter config mode if all requests matched */
   if (net->lss.match == LSS_MATCH_ALL)
   {
      uint8_t msg[8] = { 0 };

      net->lss.match = 0;

      /* Indicate success */
      LOG_DEBUG (CO_LSS_LOG, "lss identified\n");
      co_put_uint8 (msg, CS_IDENTIFY_SLAVE);
      os_channel_send (net->channel, 0x7E4, msg, sizeof(msg));
   }
}

int co_lss_rx (co_net_t * net, uint32_t id, uint8_t * msg, size_t dlc)
{
   uint8_t cmd = msg[0];

   if (id != 0x7E5)
      return -1;

   if (dlc != 8)
      return -1;

   switch (cmd)
   {
   case CS_SWITCH_GLOBAL:
      co_lss_switch_global (net, msg);
      break;
   case CS_CONFIGURE_NODE_ID:
      co_lss_configure_node_id (net, msg);
      break;
   case CS_CONFIGURE_BIT_TIMING:
      co_lss_configure_bit_timing (net, msg);
      break;
   case CS_ACTIVATE_BIT_TIMING:
      co_lss_activate_bit_timing (net, msg);
      break;
   case CS_STORE_CONFIGURATION:
      co_lss_store_configuration (net, msg);
      break;
   case CS_SWITCH_SELECTIVE_VENDOR_ID:
   case CS_SWITCH_SELECTIVE_PRODUCT_NO:
   case CS_SWITCH_SELECTIVE_REVISION_NO:
   case CS_SWITCH_SELECTIVE_SERIAL_NO:
      co_lss_switch_selective (net, msg);
      break;
   case CS_INQUIRE_IDENTITY_VENDOR_ID:
   case CS_INQUIRE_IDENTITY_PRODUCT_NO:
   case CS_INQUIRE_IDENTITY_REVISION_NO:
   case CS_INQUIRE_IDENTITY_SERIAL_NO:
   case CS_INQUIRE_IDENTITY_NODE_ID:
      co_lss_inquire_identity (net, msg);
      break;
   case CS_IDENTIFY_NON_CONFIGURED_REMOTE:
      co_lss_identify_non_configured_remote (net);
      break;
   case CS_IDENTIFY_REMOTE_VENDOR_ID:
   case CS_IDENTIFY_REMOTE_PRODUCT_NO:
   case CS_IDENTIFY_REMOTE_REVISION_LOW:
   case CS_IDENTIFY_REMOTE_REVISION_HIGH:
   case CS_IDENTIFY_REMOTE_SERIAL_LOW:
   case CS_IDENTIFY_REMOTE_SERIAL_HIGH:
      co_lss_identify_remote (net, msg);
      break;
   default:
      LOG_WARNING (CO_LSS_LOG, "unknown lss cmd %d\n", cmd);
      break;
   }

   return 0;
}

uint8_t co_lss_get_persistent_node_id (co_net_t * net)
{
   void * arg;
   uint8_t node;

   if (net->open == NULL || net->read == NULL || net->close == NULL)
      goto error1;

   arg = net->open (CO_STORE_LSS);
   if (arg == NULL)
      goto error1;

   /* Get persistent node id */
   if (net->read (arg, &node, sizeof(node)) < 0)
      goto error2;

   /* Node-ID 0 is invalid */
   if (node == 0)
      goto error2;

   if (net->close (arg) < 0)
      goto error1;

   return node;

 error2:
   net->close (arg);
 error1:
   return net->node;
}

int co_lss_get_persistent_bitrate (co_net_t * net)
{
   void * arg;
   uint8_t node;
   int bitrate;

   if (net->open == NULL || net->read == NULL || net->close == NULL)
      goto error1;

   arg = net->open (CO_STORE_LSS);
   if (arg == NULL)
      goto error1;

   /* Get persistent node id */
   if (net->read (arg, &node, sizeof(node)) < 0)
      goto error2;

   /* Node-ID 0 is invalid */
   if (node == 0)
      goto error2;

   /* Get persistent bitrate */
   if (net->read (arg, &bitrate, sizeof(bitrate)) < 0)
      goto error2;

   if (net->close (arg) < 0)
      goto error1;

   return bitrate;

 error2:
   net->close (arg);
 error1:
   return net->bitrate;
}


void co_lss_init (co_net_t * net)
{
   net->lss.state = LSS_STATE_WAITING;
   net->lss.identity = co_obj_find (net, 0x1018);
   net->lss.match = 0;
}
