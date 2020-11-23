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

#include "co_heartbeat.h"
#include "test_util.h"

// Test fixture

class HeartbeatTest : public TestBase
{
};

// Tests

TEST_F (HeartbeatTest, OD1017)
{
   uint64_t value;
   uint32_t result;

   value = 1000;

   result = co_od1017_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (net.hb_time, value);

   net.hb_time = 2000;

   result = co_od1017_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (value, net.hb_time);
}

TEST_F (HeartbeatTest, OD1016)
{
   uint64_t value;
   uint32_t result;

   // Monitor node 1
   value  = (1 << 16) | 1000;
   result = co_od1016_fn (&net, OD_EVENT_WRITE, NULL, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (1u, net.heartbeat[0].node);
   EXPECT_EQ (1000u, net.heartbeat[0].time);

   // Duplicate entry for node 1, should fail
   value  = (1 << 16) | 2000;
   result = co_od1016_fn (&net, OD_EVENT_WRITE, NULL, NULL, 2, &value);
   EXPECT_EQ (CO_SDO_ABORT_PARAM_INCOMPATIBLE, result);

   // Read back previously written value
   value  = 0;
   result = co_od1016_fn (&net, OD_EVENT_READ, NULL, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ ((1 << 16) | 1000u, value);
}

TEST_F (HeartbeatTest, HeartbeatProducer)
{
   uint8_t expected[] = {127, 4, 5};

   net.hb_time = 1000;

   // Timer has expired, should send heartbeat
   co_heartbeat_timer (&net, 1000 * 1000);
   EXPECT_EQ (1u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x701, &expected[0], 1));

   net.state = STATE_STOP;

   // Timer has not expired, should not send heartbeat
   co_heartbeat_timer (&net, 1500 * 1000);
   EXPECT_EQ (1u, mock_os_channel_send_calls);

   // Timer has expired, should send heartbeat
   co_heartbeat_timer (&net, 2000 * 1000);
   EXPECT_EQ (2u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x701, &expected[1], 1));

   net.state = STATE_OP;

   // Timer has expired, should send heartbeat
   co_heartbeat_timer (&net, 3000 * 1000);
   EXPECT_EQ (3u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x701, &expected[2], 1));
}

TEST_F (HeartbeatTest, HeartbeatConsumer)
{
   uint8_t heartbeat = 0x01;

   net.heartbeat[0].node = 1;
   net.heartbeat[0].time = 1000;

   // Receive heartbeat within timer window
   mock_os_get_current_time_us_result = 500 * 1000;
   co_heartbeat_rx (&net, 1, &heartbeat, 1);
   EXPECT_TRUE (net.heartbeat[0].is_alive);

   // Timer has expired, should send EMCY
   co_heartbeat_timer (&net, 1500 * 1000);
   EXPECT_FALSE (net.heartbeat[0].is_alive);
   EXPECT_EQ (1u, mock_co_emcy_tx_calls);

   // Timer already expired, should not send EMCY
   co_heartbeat_timer (&net, 1600 * 1000);
   EXPECT_FALSE (net.heartbeat[0].is_alive);
   EXPECT_EQ (1u, mock_co_emcy_tx_calls);

   // Receive heartbeat, should set is_alive
   mock_os_get_current_time_us_result = 2000 * 1000;
   co_heartbeat_rx (&net, 1, &heartbeat, 1);
   EXPECT_TRUE (net.heartbeat[0].is_alive);
}
