/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * http://www.rt-labs.com
 * Copyright 2017 rt-labs AB, Sweden.
 * See LICENSE file in the project root for full license information.
 ********************************************************************/

#include "co_mngr.h"
#include "co_api.h"
#include "co_obj.h"
#include "co_nmt.h"
#include "co_node_guard.h"
#include "options.h"
#include "osal.h"
#include <gtest/gtest.h>

#include "mocks.h"
#include "test_util.h"

#if CO_CONF_MNGR > 0

#define CALL_AND_CHECK(function, event, item) \
      ret = function(client.net, event, NULL, NULL, i + 1, &tmp); \
      EXPECT_EQ(0u, ret); \
      EXPECT_EQ(net.item[i], tmp);

#define TEST_OD1Fxx(test, item) \
TEST_F(MngrTest, test) \
{ \
   uint32_t ret, tmp; \
   uint8_t i; \
   resetNmgrOD (&client); \
   for (i = 0; i < CO_CONF_MNGR; i++) \
   { \
      net.item[i] = 0xAAAAAAAA; \
      CALL_AND_CHECK(co_##test, OD_EVENT_READ, item); \
      tmp = 0x55555555; \
      CALL_AND_CHECK(co_##test, OD_EVENT_WRITE, item); \
      CALL_AND_CHECK(co_##test, OD_EVENT_READ, item); \
   } \
}

// Test fixture

static const co_obj_t odtest_od[] =
{
   { 0x102A, OTYPE_VAR,   0,                OD102A, co_od102a_fn },
   { 0x1F26, OTYPE_ARRAY, CO_CONF_MNGR,     OD1F26, co_od1f26_fn },
   { 0x1F27, OTYPE_ARRAY, CO_CONF_MNGR,     OD1F27, co_od1f27_fn },
   { 0x1F80, OTYPE_VAR,   0,                OD1F80, co_od1f80_fn },
   { 0x1F81, OTYPE_ARRAY, CO_CONF_MNGR,     OD1F81, co_od1f81_fn },
   { 0x1F82, OTYPE_ARRAY, CO_CONF_MNGR + 1, OD1F82, co_od1f82_fn },
   { 0x1F83, OTYPE_ARRAY, CO_CONF_MNGR + 1, OD1F83, co_od1f83_fn },
   { 0x1F84, OTYPE_ARRAY, CO_CONF_MNGR,     OD1F84, co_od1f84_fn },
   { 0x1F85, OTYPE_ARRAY, CO_CONF_MNGR,     OD1F85, co_od1f85_fn },
   { 0x1F86, OTYPE_ARRAY, CO_CONF_MNGR,     OD1F86, co_od1f86_fn },
   { 0x1F87, OTYPE_ARRAY, CO_CONF_MNGR,     OD1F87, co_od1f87_fn },
   { 0x1F88, OTYPE_ARRAY, CO_CONF_MNGR,     OD1F88, co_od1f88_fn },
   { 0x1F89, OTYPE_ARRAY, CO_CONF_MNGR,     OD1F89, co_od1f89_fn },
   { 0, OTYPE_VAR, 0, NULL, NULL},
};

static void _co_mngr_cb_nmt (void * arg, co_state_t state)
{
   return;
}

class MngrTest : public ::testing::Test
{
protected:
   virtual void SetUp() {
      client.net = &net;
      net.od = odtest_od;
      net.open = store_open;
      net.read = store_read;
      net.write = store_write;
      net.close = store_close;
      net.cb_arg = NULL;
      net.cb_nmt = _co_mngr_cb_nmt;
      net.cb_reset = NULL;
      net.cb_emcy = NULL;
      net.number_of_errors = 0;
      net.emcy.error = 0;
      net.error_behavior = 0;
      co_nmt_init(&net);
   };

   co_client_t client;
   co_net_t net;

};

// Tests

static void resetNmgrOD (co_client_t * client)
{
   co_net_t * net = client->net;

   int i;
   for (i = 0; i < CO_CONF_MNGR; i++)
   {
      net->conf_exp_conf_date[i] = 0;
      net->conf_exp_conf_time[i] = 0;
      net->conf_exp_conf_time[i] = 0;
      net->nmt_slave_assignment[i] = 0;
      net->nmt_request[i] = 0;
      net->nmt_request_node_guard[i] = 0;
      net->nmt_node_ts_resp[i] = 0;
      net->nmt_node_ts_req[i] = 0;
      net->nmt_slave_device_type[i] = 0;
      net->nmt_slave_vendor_id[i] = 0;
      net->nmt_slave_prod_code[i] = 0;
      net->nmt_slave_rev_num[i] = 0;
      net->nmt_slave_ser_num[i] = 0;
      net->nmt_slave_boot_time[i] = 0;
   }

   net->nmt_inhibit = 0;
   net->nmt_startup = 0;
}

TEST_F(MngrTest, od102a_fn)
{
   uint32_t ret;
   uint32_t tmp;

   resetNmgrOD (&client);

   net.nmt_inhibit= 100;

   ret = co_od102a_fn(client.net, OD_EVENT_READ, NULL, NULL, 0, &tmp);
   EXPECT_EQ(0u, ret);
   EXPECT_EQ(net.nmt_inhibit, tmp);

   tmp = 500;
   ret = co_od102a_fn(client.net, OD_EVENT_WRITE, NULL, NULL, 0, &tmp);
   EXPECT_EQ(0u, ret);
   EXPECT_EQ(net.nmt_inhibit, tmp);

   ret = co_od102a_fn(client.net, OD_EVENT_READ, NULL, NULL, 0, &tmp);
   EXPECT_EQ(0u, ret);
   EXPECT_EQ(net.nmt_inhibit, tmp);
}

TEST_OD1Fxx(od1f26_fn, conf_exp_conf_date)
TEST_OD1Fxx(od1f27_fn, conf_exp_conf_time)

TEST_F(MngrTest, od1f80_fn)
{
   uint32_t ret;
   uint32_t tmp;

   resetNmgrOD (&client);

   net.nmt_startup = 100;

   ret = co_od1f80_fn(client.net, OD_EVENT_READ, NULL, NULL, 0, &tmp);
   EXPECT_EQ(0u, ret);
   EXPECT_EQ(net.nmt_startup, tmp);

   net.nmt_startup = 0;

   tmp = CO_MNGR_NMT_STARTUP_SUPPORT;
   ret = co_od1f80_fn(client.net, OD_EVENT_WRITE, NULL, NULL, 0, &tmp);
   EXPECT_EQ(0u, ret);
   EXPECT_EQ(net.nmt_startup, (uint32_t) CO_MNGR_NMT_STARTUP_SUPPORT);

   ret = co_od1f80_fn(client.net, OD_EVENT_READ, NULL, NULL, 0, &tmp);
   EXPECT_EQ(0u, ret);
   EXPECT_EQ(net.nmt_startup, tmp);

   net.nmt_startup = 0;

   tmp = 0xFF;
   ret = co_od1f80_fn(client.net, OD_EVENT_WRITE, NULL, NULL, 0, &tmp);
   EXPECT_EQ(CO_SDO_ABORT_VALUE, ret);
   EXPECT_EQ(net.nmt_startup, 0u);

   ret = co_od1f80_fn(client.net, OD_EVENT_READ, NULL, NULL, 0, &tmp);
   EXPECT_EQ(0u, ret);
   EXPECT_EQ(net.nmt_startup, tmp);
}

TEST_OD1Fxx(od1f81_fn, nmt_slave_assignment)

TEST_F(MngrTest, od1f82_fn)
{
   uint32_t ret;
   uint32_t tmp;
   uint8_t i,n;

   resetNmgrOD (&client);
   co_nmt_init(client.net);

   for (i = 0; i < CO_CONF_MNGR; i++)
   {
      net.nmt_request[i] = 100;

      ret = co_od1f82_fn(client.net, OD_EVENT_READ, NULL, NULL, i + 1, &tmp);
      EXPECT_EQ(0u, ret);
      EXPECT_EQ(net.nmt_request[i], tmp);
   }

   tmp = 0xFF;
   ret = co_od1f82_fn(client.net, OD_EVENT_READ, NULL, NULL, 128, &tmp);
   EXPECT_EQ(CO_SDO_ABORT_ACCESS_WO, ret);

   for (i = 0; i < CO_CONF_MNGR; i++)
   {
      net.nmt_request[i] = 0;

      for (n = 0; n < 255; n++)
      {
         mock_os_channel_send_id = 0;

         tmp = n;

         ret = co_od1f82_fn(client.net, OD_EVENT_WRITE, NULL, NULL, i + 1, &tmp);
         if (((n >= 4) && (n <= 7)) || (n == 127))
         {
            EXPECT_EQ(0u, ret);
            EXPECT_EQ((uint32_t) (CO_FUNCTION_NMT + i + 1), mock_os_channel_send_id);
            EXPECT_EQ(2u, mock_os_channel_send_dlc);
            switch (tmp)
            {
               case 5:     EXPECT_EQ(CO_NMT_OPERATIONAL, mock_os_channel_send_data[0]);            break;
               case 4:     EXPECT_EQ(CO_NMT_STOPPED, mock_os_channel_send_data[0]);                break;
               case 127:   EXPECT_EQ(CO_NMT_PRE_OPERATIONAL, mock_os_channel_send_data[0]);        break;
               case 6:     EXPECT_EQ(CO_NMT_RESET_NODE, mock_os_channel_send_data[0]);             break;
               case 7:     EXPECT_EQ(CO_NMT_RESET_COMMUNICATION, mock_os_channel_send_data[0]);    break;
               default:    FAIL();                                                                 break;
            }
            EXPECT_EQ(i + 1, mock_os_channel_send_data[1]);
         }
         else
         {
            EXPECT_EQ(CO_SDO_ABORT_VALUE, ret);
            EXPECT_EQ(0u, mock_os_channel_send_id);
         }
      }
   }

   for (n = 0; n < 255; n++)
   {
      mock_os_channel_send_id = 0;

      tmp = n;

      ret = co_od1f82_fn(client.net, OD_EVENT_WRITE, NULL, NULL, 128, &tmp);
      if (((n >= 4) && (n <= 7)) || (n == 127))
      {
         EXPECT_EQ(0u, ret);
         EXPECT_EQ((uint32_t) CO_FUNCTION_NMT, mock_os_channel_send_id);
         EXPECT_EQ(2u, mock_os_channel_send_dlc);
         switch (tmp)
         {
            case 5:
            {
               EXPECT_EQ(CO_NMT_OPERATIONAL, mock_os_channel_send_data[0]);
               EXPECT_EQ(STATE_OP, net.state);
               break;
            }
            case 4:
            {
               EXPECT_EQ(CO_NMT_STOPPED, mock_os_channel_send_data[0]);
               EXPECT_EQ(STATE_STOP, net.state);
               break;
            }
            case 127:
            {
               EXPECT_EQ(CO_NMT_PRE_OPERATIONAL, mock_os_channel_send_data[0]);
               EXPECT_EQ(STATE_PREOP, net.state);
               break;
            }
            case 6:
            {
               EXPECT_EQ(CO_NMT_RESET_NODE, mock_os_channel_send_data[0]);
               EXPECT_EQ(STATE_PREOP, net.state);
               break;
            }
            case 7:
            {
               EXPECT_EQ(CO_NMT_RESET_COMMUNICATION, mock_os_channel_send_data[0]);
               EXPECT_EQ(STATE_PREOP, net.state);
               break;
            }
            default:    FAIL();                                                                 break;
         }
         EXPECT_EQ(0, mock_os_channel_send_data[1]);
      }
      else
      {
         EXPECT_EQ(CO_SDO_ABORT_VALUE, ret);
         EXPECT_EQ(0u, mock_os_channel_send_id);
      }
   }
}

TEST_F(MngrTest, od1f83_fn)
{
   uint32_t ret;
   uint32_t tmp;
   uint8_t i;

   resetNmgrOD (&client);

   for (i = 0; i < CO_CONF_MNGR; i++)
   {
      net.nmt_request_node_guard[i] = 100;

      ret = co_od1f83_fn(client.net, OD_EVENT_READ, NULL, NULL, i + 1, &tmp);
      EXPECT_EQ(0u, ret);
      EXPECT_EQ(net.nmt_request_node_guard[i], tmp);
      net.nmt_request_node_guard[i] = 0;
   }

   tmp = 0xFF;
   ret = co_od1f83_fn(client.net, OD_EVENT_READ, NULL, NULL, 128, &tmp);
   EXPECT_EQ(CO_SDO_ABORT_ACCESS_WO, ret);

   for (i = 0; i < CO_CONF_MNGR; i++)
   {
      uint32_t _start_us = os_get_current_time_us();
      tmp = 1;
      ret = co_od1f83_fn(client.net, OD_EVENT_WRITE, NULL, NULL, i + 1, &tmp);
      EXPECT_EQ(0u, ret);
      EXPECT_EQ(net.nmt_request_node_guard[i], tmp);

      ret = co_od1f83_fn(client.net, OD_EVENT_READ, NULL, NULL, i + 1, &tmp);
      EXPECT_EQ(0u, ret);
      EXPECT_EQ(net.nmt_request_node_guard[i], tmp);

      EXPECT_LE(_start_us, net.nmt_node_ts_resp[i]);
      EXPECT_LE(net.nmt_node_ts_resp[i], os_get_current_time_us());
   }
}

TEST_F(MngrTest, remote_node_guarding)
{
   uint32_t ret;
   uint32_t tmp;
   uint8_t i;
   uint32_t _now;
   uint8_t expected[][8] = {
      { 0x30, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
   };   
   
   resetNmgrOD (&client);
   co_nmt_init(client.net);
   net.state = STATE_OP;
   net.emcy.cobid = 0x81u;
   net.node_guard.is_alive = 0;

   /* Set guard time from node 2 */
   net.nmt_slave_assignment[1] = (10 << 16) + (5 << 8);

   mock_co_nmt_rtr_calls = 0;
   mock_os_channel_send_id = 0;
   
   /* Setup node guarding on node 2 */
   tmp = 1;
   ret = co_od1f83_fn(client.net, OD_EVENT_WRITE, NULL, NULL, 2, &tmp);
   EXPECT_EQ(0u, ret);
   EXPECT_EQ(net.nmt_request_node_guard[1], tmp);
   
   /* workaround since os_get_current_time_us is not reliable in windows */
   _now = os_get_current_time_us();
   _now += 1 * 1000;
   EXPECT_EQ(STATE_OP, net.state);

   /* execute co_node_guard_timer */
   co_node_guard_timer(client.net, _now);
   EXPECT_EQ(mock_co_nmt_rtr_calls, 1u);

   for (i = 1; i < 5; i++)
   {
      _now += 101 * 100;
      /* execute co_node_guard_timer */
      co_node_guard_timer(client.net, _now);
      EXPECT_EQ(mock_co_nmt_rtr_calls, (uint32_t) (i + 1));
      EXPECT_EQ((uint32_t) (CO_RTR_MASK + CO_FUNCTION_NMT_ERR + 2), mock_os_channel_send_id);
      EXPECT_EQ(1u, mock_os_channel_send_dlc);
      EXPECT_EQ(STATE_OP, net.state);
   }

   EXPECT_EQ(STATE_OP, net.state);

   _now += 10 * 1000;
   
   /* execute co_node_guard_timer */
   co_node_guard_timer(client.net, _now);
   EXPECT_EQ(mock_co_nmt_rtr_calls, 5u);

   EXPECT_EQ(0x81u, mock_os_channel_send_id);
   EXPECT_EQ(8u, mock_os_channel_send_dlc);
   EXPECT_TRUE(ArraysMatch(expected[0], mock_os_channel_send_data));
   EXPECT_EQ(STATE_PREOP, net.state);
}

TEST_OD1Fxx(od1f84_fn, nmt_slave_device_type)
TEST_OD1Fxx(od1f85_fn, nmt_slave_vendor_id)
TEST_OD1Fxx(od1f86_fn, nmt_slave_prod_code)
TEST_OD1Fxx(od1f87_fn, nmt_slave_rev_num)
TEST_OD1Fxx(od1f88_fn, nmt_slave_ser_num)
TEST_OD1Fxx(od1f89_fn, nmt_slave_boot_time)
#endif
