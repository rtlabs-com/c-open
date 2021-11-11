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

#include "co_nmt.h"
#include "test_util.h"

// Test fixture

class NmtTest : public TestBase
{
 protected:
   virtual void SetUp()
   {
      TestBase::SetUp();
   }
};

// Tests

TEST_F (NmtTest, Fsm)
{
   uint8_t command[][8] = {
      {CO_NMT_OPERATIONAL, 0x01},
      {CO_NMT_STOPPED, 0x01},
      {CO_NMT_OPERATIONAL, 0x01},
      {CO_NMT_PRE_OPERATIONAL, 0x01},
      {CO_NMT_RESET_NODE, 0x01},
      {CO_NMT_RESET_COMMUNICATION, 0x01},
      {CO_NMT_OPERATIONAL, 0x00},
   };

   // Poweron, should go to PREOP
   co_nmt_init (&net);
   EXPECT_EQ (STATE_PREOP, net.state);
   EXPECT_EQ (4u, cb_nmt_calls);
   EXPECT_EQ (3u, mock_co_od_reset_calls);
   EXPECT_EQ (1u, mock_os_channel_send_calls);

   // Operational, should go to OP
   co_nmt_rx (&net, 0, command[0], 2);
   EXPECT_EQ (STATE_OP, net.state);
   EXPECT_EQ (5u, cb_nmt_calls);

   // Stopped, should go to STOP
   co_nmt_rx (&net, 0, command[1], 2);
   EXPECT_EQ (STATE_STOP, net.state);
   EXPECT_EQ (6u, cb_nmt_calls);

   // Operational, should go to OP
   co_nmt_rx (&net, 0, command[2], 2);
   EXPECT_EQ (STATE_OP, net.state);
   EXPECT_EQ (7u, cb_nmt_calls);

   // Pre operational, should go to PREOP
   co_nmt_rx (&net, 0, command[3], 2);
   EXPECT_EQ (STATE_PREOP, net.state);
   EXPECT_EQ (8u, cb_nmt_calls);

   // Reset node, should go to PREOP
   co_nmt_rx (&net, 0, command[4], 2);
   EXPECT_EQ (STATE_PREOP, net.state);
   EXPECT_EQ (11u, cb_nmt_calls);
   EXPECT_EQ (1u, cb_reset_calls);
   EXPECT_EQ (6u, mock_co_od_reset_calls);
   EXPECT_EQ (2u, mock_os_channel_send_calls);

   // Reset communication, should go to PREOP
   co_nmt_rx (&net, 0, command[5], 2);
   EXPECT_EQ (STATE_PREOP, net.state);
   EXPECT_EQ (13u, cb_nmt_calls);
   EXPECT_EQ (1u, cb_reset_calls);
   EXPECT_EQ (7u, mock_co_od_reset_calls);
   EXPECT_EQ (3u, mock_os_channel_send_calls);

   // Broadcast operational, should go to OP
   co_nmt_rx (&net, 0, command[6], 2);
   EXPECT_EQ (STATE_OP, net.state);
   EXPECT_EQ (14u, cb_nmt_calls);
}

TEST_F (NmtTest, BadNMT)
{
   uint8_t command[][8] = {
      {0x42, 0x01},
      {CO_NMT_OPERATIONAL, 0x03},
   };

   // Bad NMT command, should stay in PREOP
   co_nmt_rx (&net, 0, command[0], 2);
   EXPECT_EQ (STATE_PREOP, net.state);
   EXPECT_EQ (0u, cb_nmt_calls);

   // Bad node id
   co_nmt_rx (&net, 0, command[1], 2);
   EXPECT_EQ (STATE_PREOP, net.state);
   EXPECT_EQ (0u, cb_nmt_calls);

   // Bad dlc
   co_nmt_rx (&net, 0, command[0], 1);
   EXPECT_EQ (STATE_PREOP, net.state);
   EXPECT_EQ (0u, cb_nmt_calls);

   // Bad id
   co_nmt_rx (&net, 1, command[0], 1);
   EXPECT_EQ (STATE_PREOP, net.state);
   EXPECT_EQ (0u, cb_nmt_calls);
}

TEST_F (NmtTest, LssFsm)
{
   net.lss.node = 0xFF;

   // Stay in STATE_INIT_COMM if lss node id is invalid
   co_nmt_event (&net, EVENT_RESETCOMM);
   EXPECT_EQ (STATE_INIT_COMM, net.state);
}
