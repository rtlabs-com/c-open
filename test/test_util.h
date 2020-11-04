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

#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <gtest/gtest.h>
#include "mocks.h"
#include "co_obj.h"
#include "co_nmt.h"
#include "options.h"
#include "co_od.h"
#include "co_pdo.h"

extern "C" uint32_t cb2001 (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

class TestBase : public ::testing::Test
{
 protected:
   virtual void SetUp()
   {
      memset (&net, 0, sizeof (net));
      store_init();

      net.node      = 1;
      net.od        = test_od;
      net.cb_emcy   = cb_emcy;
      net.cb_reset  = cb_reset;
      net.cb_nmt    = cb_nmt;
      net.cb_sync   = cb_sync;
      net.cb_notify = cb_notify;
      net.open      = store_open;
      net.read      = store_read;
      net.write     = store_write;
      net.close     = store_close;

      co_pdo_init (&net);
      co_nmt_init (&net);
      co_od_reset (&net, CO_STORE_COMM, 0x1000, 0x1FFF);

      cb_emcy_calls   = 0;
      cb_reset_calls  = 0;
      cb_nmt_calls    = 0;
      cb_sync_calls   = 0;
      cb_notify_calls = 0;

      mock_os_get_current_time_us_result = 0;
      mock_os_channel_send_calls         = 0;
      mock_os_channel_send_id            = 0;
      mock_os_channel_receive_calls      = 0;
      mock_os_channel_bus_off_calls      = 0;
      mock_os_channel_bus_on_calls       = 0;
      mock_os_channel_set_bitrate_calls  = 0;
      mock_os_channel_set_filter_calls   = 0;
      mock_os_channel_get_state_calls    = 0;
      mock_co_od_reset_calls             = 0;
      mock_co_emcy_tx_calls              = 0;
      store_open_calls                   = 0;

      strcpy (name1008, "new slave");
      OD1008[0].bitlength = 8 * strlen (name1008);

      strcpy (name100A, "123");
      OD100A[0].bitlength = 8 * strlen (name100A);
   }

   co_net_t net;

   const co_entry_t OD1000[1] = {
      {0, OD_RO, DTYPE_UNSIGNED32, 32, 0x00420192, NULL},
   };

   char name1008[20];
   co_entry_t OD1008[1] = {
      {0, OD_RW, DTYPE_VISIBLE_STRING, 8 * sizeof (name1008), 0, name1008},
   };

   char name1009[20]    = {0};
   co_entry_t OD1009[1] = {
      {0, OD_RW, DTYPE_VISIBLE_STRING, 8 * sizeof (name1009), 0, name1009},
   };

   char name100A[20];
   co_entry_t OD100A[1] = {
      {0, OD_RW, DTYPE_VISIBLE_STRING, 8 * sizeof (name100A), 0, name100A},
   };

   const co_entry_t OD1018[5] = {
      {0, OD_RO, DTYPE_UNSIGNED8, 8, 4, NULL},
      {1, OD_RO, DTYPE_UNSIGNED32, 32, 1, NULL},
      {2, OD_RO, DTYPE_UNSIGNED32, 32, 2, NULL},
      {3, OD_RO, DTYPE_UNSIGNED32, 32, 3, NULL},
      {4, OD_RO, DTYPE_UNSIGNED32, 32, 4, NULL},
   };

   uint32_t arr2000[8]        = {1, 2, 3, 4, 5, 6, 7, 8};
   const co_entry_t OD2000[2] = {
      {0x00, OD_RO, DTYPE_UNSIGNED8, 8, 8, NULL},
      {0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0, arr2000},
   };

   const co_entry_t OD2001[3] = {
      {0x00, OD_RW, DTYPE_UNSIGNED8, 8, 2, NULL},
      {0x01, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL},
      {0x02, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL},
   };

   uint32_t value6000         = 0x12345678;
   const co_entry_t OD6000[1] = {
      {0, OD_NOTIFY | OD_RO | OD_TPDO, DTYPE_UNSIGNED32, 32, 0, &value6000},
   };

   uint16_t value6003_07;
   uint8_t value6003_08;
   uint32_t value6003_09;
   uint8_t value6003_0A;
   uint64_t value6003_0B;
   const co_entry_t OD6003[12] = {
      {0x00, OD_RW, DTYPE_UNSIGNED8, 8, 0x0B, NULL},
      // Pack
      {0x01, OD_RW, DTYPE_UNSIGNED16, 16, 4, NULL},
      {0x02, OD_RW, DTYPE_UNSIGNED8, 8, 99, NULL},
      // PackLarge
      {0x03, OD_RW, DTYPE_UNSIGNED32, 32, 4, NULL},
      {0x04, OD_RW, DTYPE_UNSIGNED8, 8, 99, NULL},
      // PackSmall
      {0x05, OD_RW, DTYPE_BOOLEAN, 1, 0, NULL},
      {0x06, OD_RW, DTYPE_BOOLEAN, 1, 1, NULL},
      // Unpack
      {0x07, OD_RW, DTYPE_UNSIGNED16, 16, 0, &value6003_07},
      {0x08, OD_RW, DTYPE_UNSIGNED8, 8, 0, &value6003_08},
      // UnpackLarge
      {0x09, OD_RW, DTYPE_UNSIGNED32, 32, 0, &value6003_09},
      {0x0A, OD_RW, DTYPE_UNSIGNED8, 8, 0, &value6003_0A},
      // Largest
      {0x0B, OD_RW, DTYPE_UNSIGNED64, 64, 0, &value6003_0B},
   };

   uint32_t value7000;
   const co_entry_t OD7000[1] = {
      {0, OD_RW | OD_RPDO, DTYPE_UNSIGNED32, 32, 0, &value7000},
   };

   const co_obj_t test_od[36] = {
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
      {0x1018, OTYPE_RECORD, 4,               OD1018, NULL},
      {0x1017, OTYPE_VAR,    0,               OD1017, co_od1017_fn},
      {0x1019, OTYPE_VAR,    0,               OD1019, co_od1019_fn},
      {0x1020, OTYPE_ARRAY,  2,               OD1020, co_od1020_fn},
      {0x1028, OTYPE_ARRAY,  MAX_EMCY_COBIDS, OD1028, co_od1028_fn},
      {0x1029, OTYPE_ARRAY,  1,               OD1029, co_od1029_fn},
      {0x1400, OTYPE_RECORD, 5,               OD1400, co_od1400_fn},
      {0x1533, OTYPE_RECORD, 5,               OD1400, co_od1400_fn},
      {0x1600, OTYPE_RECORD, MAX_PDO_ENTRIES, OD1600, co_od1600_fn},
      {0x1733, OTYPE_RECORD, MAX_PDO_ENTRIES, OD1600, co_od1600_fn},
      {0x1800, OTYPE_RECORD, 6,               OD1800, co_od1800_fn},
      {0x1899, OTYPE_RECORD, 6,               OD1800, co_od1800_fn},
      {0x1A00, OTYPE_RECORD, MAX_PDO_ENTRIES, OD1A00, co_od1A00_fn},
      {0x1A99, OTYPE_RECORD, MAX_PDO_ENTRIES, OD1A00, co_od1A00_fn},
      {0x2000, OTYPE_ARRAY,  8,               OD2000, NULL},
      {0x2001, OTYPE_RECORD, 2,               OD2001, cb2001},
      {0x6000, OTYPE_VAR,    0,               OD6000, NULL},
      {0x6003, OTYPE_RECORD, 12,              OD6003, NULL},
      {0x7000, OTYPE_VAR,    0,               OD7000, NULL},
      {0, OTYPE_NULL, 0, NULL, NULL},
      // clang-format on
   };

   const co_obj_t * find_obj (uint16_t index)
   {
      return co_obj_find (&net, index);
   }

   const co_entry_t * find_entry (const co_obj_t * obj, uint8_t subindex)
   {
      return co_entry_find (&net, obj, subindex);
   }
};

inline std::string FormatHexInt (int value)
{
   std::stringstream ss;
   ss << std::hex << std::showbase << value;
   return ss.str();
}

inline std::string FormatByte (uint8_t value)
{
   std::stringstream ss;
   ss << std::setfill ('0') << std::setw (2) << std::hex << std::showbase
      << static_cast<unsigned int> (value);
   return ss.str();
}

template <typename T, size_t size>
::testing::AssertionResult ArraysMatch (
   const T (&expected)[size],
   const T (&actual)[size])
{
   for (size_t i (0); i < size; ++i)
   {
      if (expected[i] != actual[i])
      {
         return ::testing::AssertionFailure()
                << "actual[" << i << "] ("
                << FormatByte (static_cast<int> (actual[i])) << ") != expected["
                << i << "] (" << FormatByte (static_cast<int> (expected[i]))
                << ")";
      }
   }

   return ::testing::AssertionSuccess();
}

inline ::testing::AssertionResult CanMatch (uint32_t id, void * expected, size_t dlc)
{
   uint8_t * data = (uint8_t *)expected;

   if (id != mock_os_channel_send_id)
      return ::testing::AssertionFailure()
             << "id (" << FormatHexInt (mock_os_channel_send_id)
             << ") != expected (" << FormatHexInt (id) << ")";

   if (dlc != mock_os_channel_send_dlc)
      return ::testing::AssertionFailure() << "dlc (" << mock_os_channel_send_dlc
                                           << ") != expected (" << dlc << ")";

   for (size_t i (0); i < dlc; ++i)
   {
      if (data[i] != mock_os_channel_send_data[i])
      {
         return ::testing::AssertionFailure()
                << "actual[" << i << "] ("
                << FormatByte (mock_os_channel_send_data[i]) << ") != expected["
                << i << "] (" << FormatByte (data[i]) << ")";
      }
   }

   return ::testing::AssertionSuccess();
}

#endif /* TEST_UTIL_H */
