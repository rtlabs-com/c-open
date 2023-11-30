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

#include "co_node_guard.h"
#include "test_util.h"

#define NODE_GUARD_BASE (CO_RTR_MASK | CO_FUNCTION_NMT_ERR)

// Test fixture

class NodeGuardTest : public TestBase
{
};

// Tests

TEST_F (NodeGuardTest, od100c_fn)
{
   uint32_t result;
   uint32_t value;

   net.node_guard.guard_time = 1000;

   result = co_od100C_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (net.node_guard.guard_time, value);

   value  = 500;
   result = co_od100C_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (net.node_guard.guard_time, value);

   result = co_od100C_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (net.node_guard.guard_time, value);
}

TEST_F (NodeGuardTest, od100d_fn)
{
   uint32_t result;
   uint32_t value;

   net.node_guard.life_time_factor = 10;

   result = co_od100D_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (net.node_guard.life_time_factor, value);

   value  = 5;
   result = co_od100D_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (net.node_guard.life_time_factor, value);

   result = co_od100D_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (net.node_guard.life_time_factor, value);
}

TEST_F (NodeGuardTest, NmtRtr)
{
   uint8_t expected[] = {0x04, 0x84, 0x05, 0x85, 0x7f, 0xff, 0x00, 0x80};
   uint8_t rtr        = 0x00;

   net.state = STATE_STOP;

   mock_os_channel_send_id = 0;
   co_node_guard_rx (&net, NODE_GUARD_BASE | 1, &rtr, 1);
   EXPECT_TRUE (CanMatch (0x701, &expected[0], 1));

   mock_os_channel_send_id = 0;
   co_node_guard_rx (&net, NODE_GUARD_BASE | 1, &rtr, 1);
   EXPECT_TRUE (CanMatch (0x701, &expected[1], 1));

   net.state = STATE_OP;

   mock_os_channel_send_id = 0;
   co_node_guard_rx (&net, NODE_GUARD_BASE | 1, &rtr, 1);
   EXPECT_TRUE (CanMatch (0x701, &expected[2], 1));

   mock_os_channel_send_id = 0;
   co_node_guard_rx (&net, NODE_GUARD_BASE | 1, &rtr, 1);
   EXPECT_TRUE (CanMatch (0x701, &expected[3], 1));

   net.state = STATE_PREOP;

   mock_os_channel_send_id = 0;
   co_node_guard_rx (&net, NODE_GUARD_BASE | 1, &rtr, 1);
   EXPECT_TRUE (CanMatch (0x701, &expected[4], 1));

   mock_os_channel_send_id = 0;
   co_node_guard_rx (&net, NODE_GUARD_BASE | 1, &rtr, 1);
   EXPECT_TRUE (CanMatch (0x701, &expected[5], 1));

   /* faulty state should return state 0 */
   net.state = (co_state_t)42;

   mock_os_channel_send_id = 0;
   co_node_guard_rx (&net, NODE_GUARD_BASE | 1, &rtr, 1);
   EXPECT_TRUE (CanMatch (0x701, &expected[6], 1));

   mock_os_channel_send_id = 0;
   co_node_guard_rx (&net, NODE_GUARD_BASE | 1, &rtr, 1);
   EXPECT_TRUE (CanMatch (0x701, &expected[7], 1));
}

TEST_F (NodeGuardTest, HeartBeatOverNmtRtr)
{
   uint8_t rtr = 0x00;

   /* Activate heart beat timer, shall disable nmtrtr */
   net.hb_time = 1000;

   mock_os_channel_send_id = 0;
   co_node_guard_rx (&net, NODE_GUARD_BASE | 1, &rtr, 1);
   EXPECT_NE (0x701u, mock_os_channel_send_id);
}

TEST_F (NodeGuardTest, Expire)
{
   uint8_t expected[] = {0x05, 0x85, 0x7f, 0xff, 0x30};
   uint8_t emcy[]     = {0x30, 0x81, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00};
   uint8_t rtr        = 0x00;

   net.state               = STATE_OP;
   net.error_behavior      = 0;
   net.emcy.cobid          = 0x81u;
   net.node_guard.is_alive = false;

   net.node_guard.guard_time       = 100;
   net.node_guard.life_time_factor = 2;

   // Receive node guard request, reset timestamp
   mock_os_tick_current_result = 10 * 1000;
   mock_os_channel_send_id            = 0;
   co_node_guard_rx (&net, NODE_GUARD_BASE | 1, &rtr, 1);
   EXPECT_TRUE (CanMatch (0x701, &expected[0], 1));
   EXPECT_TRUE (net.node_guard.is_alive);
   EXPECT_EQ (STATE_OP, net.state);

   // Receive node guard request, reset timestamp
   mock_os_tick_current_result = 20 * 1000;
   mock_os_channel_send_id            = 0;
   co_node_guard_rx (&net, NODE_GUARD_BASE | 1, &rtr, 1);
   EXPECT_TRUE (CanMatch (0x701, &expected[1], 1));
   EXPECT_TRUE (net.node_guard.is_alive);
   EXPECT_EQ (STATE_OP, net.state);

   // Should be alive, has not expired
   mock_os_channel_send_id = 0;
   co_node_guard_timer (&net, 50 * 1000);
   EXPECT_NE (0x81u, mock_os_channel_send_id);
   EXPECT_TRUE (net.node_guard.is_alive);
   EXPECT_EQ (STATE_OP, net.state);

   // Should be alive, has not expired
   mock_os_channel_send_id = 0;
   co_node_guard_timer (&net, 100 * 1000);
   EXPECT_NE (0x81u, mock_os_channel_send_id);
   EXPECT_TRUE (net.node_guard.is_alive);
   EXPECT_EQ (STATE_OP, net.state);

   // Should not be alive, has expired
   mock_os_channel_send_id = 0;
   co_node_guard_timer (&net, 220 * 1000);
   EXPECT_TRUE (CanMatch (0x81, emcy, 8));
   EXPECT_FALSE (net.node_guard.is_alive);
   EXPECT_EQ (STATE_PREOP, net.state);

   // Receive node guard request, reset timestamp
   mock_os_tick_current_result = 300 * 1000;
   mock_os_channel_send_id            = 0;
   co_node_guard_rx (&net, NODE_GUARD_BASE | 1, &rtr, 1);
   EXPECT_TRUE (CanMatch (0x701, &expected[2], 1));
   EXPECT_TRUE (net.node_guard.is_alive);
   EXPECT_EQ (STATE_PREOP, net.state);

   // Receive node guard request, reset timestamp
   mock_os_tick_current_result = 350 * 1000;
   mock_os_channel_send_id            = 0;
   co_node_guard_rx (&net, NODE_GUARD_BASE | 1, &rtr, 1);
   EXPECT_TRUE (CanMatch (0x701, &expected[3], 1));
   EXPECT_TRUE (net.node_guard.is_alive);
   EXPECT_EQ (STATE_PREOP, net.state);
}
