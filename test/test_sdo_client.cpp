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

#include "co_sdo.h"
#include "test_util.h"

// Test fixture

class SdoClientTest : public TestBase
{
};

// Tests

TEST_F (SdoClientTest, ExpeditedUpload)
{
   co_job_t job;
   uint32_t value;

   uint8_t expected[][8] = {
      {0x40, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},
   };
   uint8_t response[][8] = {
      {0x43, 0x00, 0x10, 0x00, 0x92, 0x01, 0x42, 0x00},
   };

   job.type         = CO_JOB_SDO_READ;
   job.sdo.node     = 1;
   job.sdo.index    = 0x1000;
   job.sdo.subindex = 0;
   job.sdo.data     = (uint8_t *)&value;
   job.sdo.remain   = sizeof (value);
   job.callback     = NULL;

   mock_os_channel_send_calls = 0;

   co_sdo_issue (&net, &job);
   EXPECT_TRUE (CanMatch (0x601, expected[0], 8));

   co_sdo_tx (&net, 1, response[0], 8);
   EXPECT_EQ (0x00420192u, value);
   EXPECT_EQ (NELEMENTS (expected), mock_os_channel_send_calls);

   EXPECT_EQ (4u, job.sdo.total);
}

TEST_F (SdoClientTest, ExpeditedDownload)
{
   co_job_t job;
   uint16_t value = 0;

   uint8_t expected[][8] = {
      {0x2b, 0x40, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00},
   };
   uint8_t response[][8] = {
      {0x60, 0x40, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   job.type         = CO_JOB_SDO_WRITE;
   job.sdo.node     = 1;
   job.sdo.index    = 0x6040;
   job.sdo.subindex = 0;
   job.sdo.data     = (uint8_t *)&value;
   job.sdo.remain   = sizeof (value);
   job.callback     = NULL;

   mock_os_channel_send_calls = 0;

   co_sdo_issue (&net, &job);
   EXPECT_TRUE (CanMatch (0x601, expected[0], 8));

   co_sdo_tx (&net, 1, response[0], 8);
   EXPECT_EQ (NELEMENTS (expected), mock_os_channel_send_calls);

   EXPECT_EQ (2u, job.sdo.total);
}

TEST_F (SdoClientTest, SegmentedUpload)
{
   co_job_t job;
   uint8_t value[16];

   uint8_t expected[][8] = {
      {0x40, 0x0a, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };
   uint8_t response[][8] = {
      {0x41, 0x0a, 0x10, 0x00, 0x1c, 0x00, 0x00, 0x00},
      {0x00, 0x57, 0x68, 0x69, 0x73, 0x74, 0x6c, 0x65},
      {0x10, 0x20, 0x32, 0x2e, 0x30, 0x32, 0x2e, 0x30},
      {0x00, 0x37, 0x2e, 0x30, 0x30, 0x20, 0x31, 0x30},
      {0x11, 0x4d, 0x61, 0x79, 0x32, 0x30, 0x30, 0x36},
   };

   job.type         = CO_JOB_SDO_READ;
   job.sdo.node     = 1;
   job.sdo.index    = 0x100a;
   job.sdo.subindex = 0;
   job.sdo.data     = value;
   job.sdo.remain   = sizeof (value);
   job.callback     = NULL;

   mock_os_channel_send_calls = 0;

   co_sdo_issue (&net, &job);
   EXPECT_TRUE (CanMatch (0x601, expected[0], 8));

   for (size_t i = 0; i < NELEMENTS (response); i++)
   {
      co_sdo_tx (&net, 1, response[i], 8);
      EXPECT_EQ (0x601u, mock_os_channel_send_id);
      if (i != NELEMENTS (response) - 1)
      {
         EXPECT_TRUE (CanMatch (0x601, expected[i + 1], 8));
      }
   }
   EXPECT_EQ (NELEMENTS (expected), mock_os_channel_send_calls);

   EXPECT_EQ (16u, job.sdo.total);
}

TEST_F (SdoClientTest, SegmentedDownload)
{
   co_job_t job;
   const char * s = "hello world";

   uint8_t expected[][8] = {
      {0x21, 0x99, 0x69, 0x00, 0x0b, 0x00, 0x00, 0x00},
      {0x00, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77},
      {0x17, 0x6f, 0x72, 0x6c, 0x64, 0x00, 0x00, 0x00},
   };
   uint8_t response[][8] = {
      {0x60, 0x99, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   job.type         = CO_JOB_SDO_WRITE;
   job.sdo.node     = 1;
   job.sdo.index    = 0x6999;
   job.sdo.subindex = 0;
   job.sdo.data     = (uint8_t *)s;
   job.sdo.remain   = strlen (s);
   job.callback     = NULL;

   mock_os_channel_send_calls = 0;

   co_sdo_issue (&net, &job);
   EXPECT_TRUE (CanMatch (0x601, expected[0], 8));

   for (size_t i = 0; i < NELEMENTS (response); i++)
   {
      co_sdo_tx (&net, 1, response[i], 8);
      EXPECT_EQ (0x601u, mock_os_channel_send_id);
      if (i != NELEMENTS (response) - 1)
      {
         EXPECT_TRUE (CanMatch (0x601, expected[i + 1], 8));
      }
   }
   EXPECT_EQ (NELEMENTS (expected), mock_os_channel_send_calls);

   EXPECT_EQ (strlen (s), job.sdo.total);
}
