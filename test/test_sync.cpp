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

#include "co_sync.h"
#include "test_util.h"

// Test fixture

class SyncTest : public TestBase
{
};

// Tests

TEST_F(SyncTest, CobIdSyncMessageConfiguration)
{
   uint32_t value;
   uint32_t result;

   value = 0x40000000;
   result = co_od1005_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (0x06090030u, result);

   value = 0x40000086;
   result = co_od1005_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);

   value = 0x40000085;
   result = co_od1005_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (0x08000000u, result);

   value = 0x00000085;
   result = co_od1005_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (0x0u, result);

   value = 0x00000086;
   result = co_od1005_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);

   value = 0x40000086;
   result = co_od1005_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);

   value = 0;
   result = co_od1005_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (0x40000086u, value);
}

TEST_F(SyncTest, OD1006)
{
   uint32_t value;
   uint32_t result;

   value = 1000;
   result = co_od1006_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (net.sync.period, value);

   net.sync.period = 2000;
   result = co_od1006_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (value, net.sync.period);
}

TEST_F(SyncTest, OD1019)
{
   uint32_t value;
   uint32_t result;

   value = 240;
   result = co_od1019_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (net.sync.overflow, value);

   net.sync.overflow = 10;
   result = co_od1019_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (value, net.sync.overflow);

   // Reserved values, should fail
   value = 1;
   result = co_od1019_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (CO_SDO_ABORT_VALUE, result);
   EXPECT_EQ (10u, net.sync.overflow);

   // Reserved values, should fail
   value = 241;
   result = co_od1019_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (CO_SDO_ABORT_VALUE, result);
   EXPECT_EQ (10u, net.sync.overflow);

   // Should fail if sync producer is active
   net.sync.period = 1000;
   value = 2;
   result = co_od1019_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (CO_SDO_ABORT_WRITE_STATE_DENIED, result);
   EXPECT_EQ (10u, net.sync.overflow);
}

TEST_F(SyncTest, SyncNoCounter)
{
   co_sync_t * sync = &net.sync;

   sync->cobid = 0x40000080;
   sync->period = 100;
   sync->counter = 0;
   sync->timestamp = 0;

   co_sync_timer (&net, 100);
   EXPECT_EQ (1u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x80, NULL, 0));
   EXPECT_EQ (1u, cb_sync_calls);

   co_sync_timer (&net, 199);
   EXPECT_EQ (1u, mock_os_channel_send_calls);

   co_sync_timer (&net, 200);
   EXPECT_EQ (2u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x80, NULL, 0));
   EXPECT_EQ (2u, cb_sync_calls);

}

TEST_F(SyncTest, SyncCounter)
{
   co_sync_t * sync = &net.sync;
   uint8_t expected[] = { 0x01, 0x02, 0x03, 0x01 };

   sync->cobid = 0x40000080;
   sync->period = 100;
   sync->overflow = 3;
   sync->counter = 1;
   sync->timestamp = 0;

   co_sync_timer (&net, 100);
   EXPECT_EQ (1u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x80, &expected[0], 1));

   co_sync_timer (&net, 199);
   EXPECT_EQ (1u, mock_os_channel_send_calls);

   co_sync_timer (&net, 200);
   EXPECT_EQ (2u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x80, &expected[1], 1));

   co_sync_timer (&net, 300);
   EXPECT_EQ (3u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x80, &expected[2], 1));

   co_sync_timer (&net, 400);
   EXPECT_EQ (4u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x80, &expected[3], 1));
}
