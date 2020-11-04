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

#include "co_lss.h"
#include "test_util.h"

// Test fixture

class LssTest : public TestBase
{
 protected:
   virtual void SetUp()
   {
      TestBase::SetUp();
      co_lss_init (&net);
   }
};

// Tests

TEST_F (LssTest, SwitchGlobal)
{
   uint8_t command[][8] = {
      {0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   co_lss_rx (&net, 0x7E5, command[0], 8);
   EXPECT_EQ (LSS_STATE_CONFIG, net.lss.state);

   co_lss_rx (&net, 0x7E5, command[1], 8);
   EXPECT_EQ (LSS_STATE_WAITING, net.lss.state);

   // Invalid - should ignore
   co_lss_rx (&net, 0x7E5, command[2], 8);
   EXPECT_EQ (LSS_STATE_WAITING, net.lss.state);
}

TEST_F (LssTest, SwitchSelective)
{
   uint8_t command[][8] = {
      {0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x41, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x42, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x43, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   co_lss_rx (&net, 0x7E5, command[0], 8);
   EXPECT_EQ (0u, mock_os_channel_send_calls);

   co_lss_rx (&net, 0x7E5, command[1], 8);
   EXPECT_EQ (0u, mock_os_channel_send_calls);

   co_lss_rx (&net, 0x7E5, command[2], 8);
   EXPECT_EQ (0u, mock_os_channel_send_calls);

   co_lss_rx (&net, 0x7E5, command[3], 8);
   EXPECT_EQ (1u, mock_os_channel_send_calls);
   EXPECT_EQ (0x7E4u, mock_os_channel_send_id);

   // Send one extra matching command - should fail
   co_lss_rx (&net, 0x7E5, command[3], 8);
   EXPECT_EQ (1u, mock_os_channel_send_calls);
}

TEST_F (LssTest, SwitchSelectiveFail)
{
   uint8_t command[][8] = {
      {0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x41, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x42, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x43, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   // Send same matching command 4 times - should fail
   for (int ix = 0; ix < 4; ix++)
   {
      co_lss_rx (&net, 0x7E5, command[0], 8);
   }
   EXPECT_EQ (0u, mock_os_channel_send_calls);

   // Last command is not match - should fail
   for (int ix = 0; ix < 4; ix++)
   {
      co_lss_rx (&net, 0x7E5, command[ix], 8);
   }
   EXPECT_EQ (0u, mock_os_channel_send_calls);
}

TEST_F (LssTest, Identify)
{
   uint8_t command[][8] = {
      {0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x5E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };
   uint8_t expected[][8] = {
      {0x5A, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x5E, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   net.lss.state = LSS_STATE_CONFIG;

   co_lss_rx (&net, 0x7E5, command[0], 8);
   EXPECT_EQ (1u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x7E4, expected[0], 8));

   net.node = 0x42;
   co_lss_rx (&net, 0x7E5, command[1], 8);
   EXPECT_EQ (2u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x7E4, expected[1], 8));

   net.lss.state = LSS_STATE_WAITING;

   // Wrong state, should not respond
   co_lss_rx (&net, 0x7E5, command[1], 8);
   EXPECT_EQ (2u, mock_os_channel_send_calls);
}

TEST_F (LssTest, IdentifyRemote)
{
   uint8_t command[][8] = {
      {0x46, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x47, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x48, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x49, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x4A, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x4B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   for (int ix = 0; ix < 6; ix++)
   {
      co_lss_rx (&net, 0x7E5, command[ix], 8);
   }
   EXPECT_EQ (1u, mock_os_channel_send_calls);
}

TEST_F (LssTest, IdentifyRemoteFail)
{
   uint8_t command[][8] = {
      {0x46, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x47, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x48, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x49, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Out of range
      {0x4A, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x4B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   for (unsigned int ix = 0; ix < NELEMENTS (command); ix++)
   {
      co_lss_rx (&net, 0x7E5, command[ix], 8);
   }
   EXPECT_EQ (0u, mock_os_channel_send_calls);
}

TEST_F (LssTest, ConfigureNodeId)
{
   uint8_t command[][8] = {
      {0x11, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x11, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Bad node id
   };
   uint8_t expected[][8] = {
      {0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x11, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };
   unsigned int ix;

   net.lss.state = LSS_STATE_CONFIG;

   for (ix = 0; ix < NELEMENTS (command); ix++)
   {
      co_lss_rx (&net, 0x7E5, command[ix], 8);
      EXPECT_TRUE (CanMatch (0x7E4, expected[ix], 8));
   }

   net.lss.state = LSS_STATE_WAITING;

   // Bad state - should ignore
   EXPECT_EQ (ix, mock_os_channel_send_calls);
   co_lss_rx (&net, 0x7E5, command[0], 8);
   EXPECT_EQ (ix, mock_os_channel_send_calls);
}

TEST_F (LssTest, ConfigureBitrate)
{
   uint8_t command[][8] = {
      {0x13, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x13, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00}, // Bad ix
      {0x13, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00}, // Bad table
   };
   uint8_t expected[][8] = {
      {0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x13, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x13, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };
   unsigned int ix;

   net.lss.state = LSS_STATE_CONFIG;

   for (ix = 0; ix < NELEMENTS (command); ix++)
   {
      co_lss_rx (&net, 0x7E5, command[ix], 8);
      EXPECT_TRUE (CanMatch (0x7E4, expected[ix], 8));
   }

   net.lss.state = LSS_STATE_WAITING;

   // Bad state - should ignore
   EXPECT_EQ (ix, mock_os_channel_send_calls);
   co_lss_rx (&net, 0x7E5, command[0], 8);
   EXPECT_EQ (ix, mock_os_channel_send_calls);
}

TEST_F (LssTest, ActivateBitrate)
{
   uint8_t command[][8] = {
      {0x13, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00}, // Configure
      {0x15, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Activate
   };

   net.lss.state = LSS_STATE_CONFIG;

   co_lss_rx (&net, 0x7E5, command[0], 8);
   co_lss_rx (&net, 0x7E5, command[1], 8);

   EXPECT_EQ (1u, mock_os_channel_set_bitrate_calls);
   EXPECT_EQ (125 * 1000, mock_os_channel_set_bitrate_bitrate);
}

TEST_F (LssTest, Persistence)
{
   uint8_t command[][8] = {
      {0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Store
   };
   uint8_t expected[][8] = {
      {0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   net.lss.state   = LSS_STATE_CONFIG;
   net.lss.node    = 0x42;
   net.lss.bitrate = 125 * 1000;

   co_lss_rx (&net, 0x7E5, command[0], 8);

   EXPECT_EQ (1u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x7E4, expected[0], 8));

   EXPECT_EQ (0x42u, co_lss_get_persistent_node_id (&net));
   EXPECT_EQ (125 * 1000, co_lss_get_persistent_bitrate (&net));
}
