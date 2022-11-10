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
#include "co_od.h"

#include "options.h"
#include <gtest/gtest.h>

#include "mocks.h"

#include <sstream>
#include <locale>
#include <iostream>
#include <iomanip>

#include "test_util.h"

// Test fixture

class SdoServerTest : public TestBase
{
};

// Tests

TEST_F (SdoServerTest, ExpeditedUpload)
{
   uint8_t expected[][8] = {
      {0x43, 0x00, 0x10, 0x00, 0x92, 0x01, 0x42, 0x00},
   };
   uint8_t command[][8] = {
      {0x40, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   mock_co_obj_find_result   = find_obj (0x1000);
   mock_co_entry_find_result = find_entry (mock_co_obj_find_result, 0);

   co_sdo_rx (&net, 1, command[0], 8);
   EXPECT_TRUE (CanMatch (0x581, expected[0], 8));
}

TEST_F (SdoServerTest, ExpeditedUploadString)
{
   uint8_t expected[][8] = {
      {0x47, 0x0a, 0x10, 0x00, 0x31, 0x32, 0x33, 0x00},
   };
   uint8_t command[][8] = {
      {0x40, 0x0a, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   mock_co_obj_find_result   = find_obj (0x100A);
   mock_co_entry_find_result = find_entry (mock_co_obj_find_result, 0);

   co_sdo_rx (&net, 1, command[0], 8);
   EXPECT_TRUE (CanMatch (0x581, expected[0], 8));
}

TEST_F (SdoServerTest, ExpeditedDownload)
{
   uint8_t expected[][8] = {
      {0x60, 0x03, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00},
   };
   uint8_t command[][8] = {
      {0x23, 0x03, 0x60, 0x00, 0x78, 0x56, 0x34, 0x12},
   };

   mock_co_obj_find_result   = find_obj (0x6003);
   mock_co_entry_find_result = find_entry (mock_co_obj_find_result, 9);

   co_sdo_rx (&net, 1, command[0], 8);
   EXPECT_TRUE (CanMatch (0x581, expected[0], 8));

   EXPECT_EQ (0x12345678u, value6003_09);
}

TEST_F (SdoServerTest, SegmentedUpload)
{
   uint8_t expected[][8] = {
      {0x41, 0x08, 0x10, 0x00, 0x09, 0x00, 0x00, 0x00},
      {0x00, 0x6e, 0x65, 0x77, 0x20, 0x73, 0x6c, 0x61},
      {0x1b, 0x76, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00},
   };
   uint8_t command[][8] = {
      {0x40, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   mock_co_obj_find_result   = find_obj (0x1008);
   mock_co_entry_find_result = find_entry (mock_co_obj_find_result, 0);

   for (size_t i = 0; i < NELEMENTS (command); i++)
   {
      co_sdo_rx (&net, 1, command[i], 8);
      EXPECT_TRUE (CanMatch (0x581, expected[i], 8));
   }
}

TEST_F (SdoServerTest, SegmentedDownload)
{
   uint8_t expected[][8] = {
      {0x60, 0x09, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };
   uint8_t command[][8] = {
      {0x21, 0x09, 0x10, 0x00, 0x0e, 0x00, 0x00, 0x00},
      {0x00, 0x6e, 0x65, 0x77, 0x20, 0x73, 0x6c, 0x61},
      {0x11, 0x76, 0x65, 0x20, 0x6e, 0x61, 0x6d, 0x65},
   };

   mock_co_obj_find_result   = find_obj (0x1009);
   mock_co_entry_find_result = find_entry (mock_co_obj_find_result, 0);

   for (size_t i = 0; i < NELEMENTS (command); i++)
   {
      co_sdo_rx (&net, 1, command[i], 8);
      EXPECT_TRUE (CanMatch (0x581, expected[i], 8));
   }

   EXPECT_STREQ ("new slave name", name1009);
   EXPECT_EQ (1u, cb_notify_calls);
}

TEST_F (SdoServerTest, SegmentedDownloadCached)
{
   uint8_t expected[][8] = {
      {0x60, 0x03, 0x60, 0x0A, 0x00, 0x00, 0x00, 0x00},
      {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };
   uint8_t command[][8] = {
      {0x21, 0x03, 0x60, 0x0A, 0x08, 0x00, 0x00, 0x00},
      {0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66},
      {0x11, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   mock_co_obj_find_result   = find_obj (0x6003);
   mock_co_entry_find_result = find_entry (mock_co_obj_find_result, 11);

   for (size_t i = 0; i < NELEMENTS (command); i++)
   {
      co_sdo_rx (&net, 1, command[i], 8);
      EXPECT_TRUE (CanMatch (0x581, expected[i], 8));
   }
}

TEST_F (SdoServerTest, SegmentedTimeout)
{
   uint8_t expected[][8] = {
      {0x41, 0x08, 0x10, 0x00, 0x09, 0x00, 0x00, 0x00},
      {0x00, 0x6e, 0x65, 0x77, 0x20, 0x73, 0x6c, 0x61},
      {0x80, 0x08, 0x10, 0x00, 0x00, 0x00, 0x04, 0x05},
   };
   uint8_t command[][8] = {
      {0x40, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };
   os_tick_t now;

   mock_co_obj_find_result   = find_obj (0x1008);
   mock_co_entry_find_result = find_entry (mock_co_obj_find_result, 0);

   co_sdo_rx (&net, 1, command[0], 8);

   // Should not time out
   now = 1000 * SDO_TIMEOUT - 1;
   co_sdo_server_timer (&net, now);
   EXPECT_TRUE (CanMatch (0x581, expected[0], 8));

   mock_os_tick_current_result = now;
   co_sdo_rx (&net, 1, command[1], 8);

   // Should not time out
   now += 1000 * SDO_TIMEOUT - 1;
   co_sdo_server_timer (&net, now);
   EXPECT_TRUE (CanMatch (0x581, expected[1], 8));

   // Should abort with timeout
   now += 1;
   co_sdo_server_timer (&net, now);
   EXPECT_TRUE (CanMatch (0x581, expected[2], 8));
}

TEST_F (SdoServerTest, BadSubIndex)
{
   const co_obj_t obj    = {0, OTYPE_NULL, 0, NULL, NULL};
   uint8_t expected[][8] = {
      {0x80, 0x00, 0x10, 0x01, 0x11, 0x00, 0x09, 0x06},
   };
   uint8_t command[][8] = {
      {0x40, 0x00, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00},
   };

   mock_co_obj_find_result   = &obj;
   mock_co_entry_find_result = NULL;

   for (size_t i = 0; i < NELEMENTS (command); i++)
   {
      co_sdo_rx (&net, 1, command[i], 8);
      EXPECT_TRUE (CanMatch (0x581, expected[i], 8));
   }
}

TEST_F (SdoServerTest, SubIndexFF)
{
   uint8_t expected[][8] = {
      {0x43, 0x00, 0x10, 0xFF, 0x07, 0x07, 0x00, 0x00},
      {0x43, 0x03, 0x10, 0xFF, 0x08, 0x07, 0x00, 0x00},
   };
   uint8_t command[][8] = {
      {0x40, 0x00, 0x10, 0xFF, 0x00, 0x00, 0x00, 0x00},
      {0x40, 0x03, 0x10, 0xFF, 0x00, 0x00, 0x00, 0x00},
   };

   mock_co_obj_find_result   = find_obj (0x1000);
   mock_co_entry_find_result = NULL;
   co_sdo_rx (&net, 1, command[0], 8);
   EXPECT_TRUE (CanMatch (0x581, expected[0], 8));

   mock_co_obj_find_result   = find_obj (0x1003);
   mock_co_entry_find_result = NULL;
   co_sdo_rx (&net, 1, command[1], 8);
   EXPECT_TRUE (CanMatch (0x581, expected[1], 8));
}
