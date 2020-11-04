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

#include "co_od.h"
#include "test_util.h"

// Test fixture

int obj_sum (co_net_t * net, const co_entry_t * entry, uintptr_t arg, int sum)
{
   return sum + entry->value;
}

int cb2001_calls;
uint32_t cb2001_value;
extern "C" uint32_t cb2001 (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   cb2001_calls++;

   if (subindex == 0)
      return CO_SDO_ABORT_BAD_SUBINDEX;

   if (event == OD_EVENT_READ)
      *value = cb2001_value;
   else if (event == OD_EVENT_WRITE)
      cb2001_value = *value;
   else if (event == OD_EVENT_RESTORE)
      cb2001_value = 0;
   return 0;
}

static const co_default_t od_defaults[] = {
   {0x2000, 1, 11},
   {0x2000, 2, 22},
   {0x2000, 3, 33},
   {0x2000, 4, 44},
   {0x2000, 5, 55},
   {0x2000, 6, 66},
   {0x2000, 7, 77},
   {0x2000, 8, 88},
};

class OdTest : public TestBase
{
 protected:
   virtual void SetUp()
   {
      TestBase::SetUp();
      cb2001_calls = 0;

      arr2000[0] = 1;
      arr2000[1] = 2;
      arr2000[2] = 3;
      arr2000[3] = 4;
      arr2000[4] = 5;
      arr2000[5] = 6;
      arr2000[6] = 7;
      arr2000[7] = 8;
   }
};

// Tests

TEST_F (OdTest, ObjFind)
{
   EXPECT_EQ (&test_od[1], co_obj_find (&net, 0x1001));
   EXPECT_EQ (&test_od[2], co_obj_find (&net, 0x1003));
   EXPECT_EQ (NULL, co_obj_find (&net, 0x1234));
   EXPECT_EQ (NULL, co_obj_find (&net, 0));
}

TEST_F (OdTest, EntryFindRecord)
{
   const co_obj_t * obj = find_obj (0x1018);

   EXPECT_EQ (&obj->entries[0], co_entry_find (&net, obj, 0));
   EXPECT_EQ (&obj->entries[1], co_entry_find (&net, obj, 1));
   EXPECT_EQ (&obj->entries[2], co_entry_find (&net, obj, 2));
   EXPECT_EQ (&obj->entries[3], co_entry_find (&net, obj, 3));
   EXPECT_EQ (&obj->entries[4], co_entry_find (&net, obj, 4));
   EXPECT_EQ (NULL, co_entry_find (&net, obj, 5));
}

TEST_F (OdTest, EntryFindSparseRecord)
{
   const co_obj_t * obj = find_obj (0x1400);

   EXPECT_EQ (&obj->entries[3], co_entry_find (&net, obj, 3));
   // Subindex 4 does not exist
   EXPECT_EQ (&obj->entries[4], co_entry_find (&net, obj, 5));
   EXPECT_EQ (NULL, co_entry_find (&net, obj, 6));
}

TEST_F (OdTest, EntryFindArray)
{
   const co_obj_t * obj = find_obj (0x1003);

   EXPECT_EQ (&obj->entries[0], co_entry_find (&net, obj, 0));
   EXPECT_EQ (&obj->entries[1], co_entry_find (&net, obj, MAX_ERRORS));
   EXPECT_EQ (NULL, co_entry_find (&net, obj, MAX_ERRORS + 1));
}

TEST_F (OdTest, EntryFindVar)
{
   const co_obj_t * obj = &test_od[0];

   EXPECT_EQ (0, co_entry_find (&net, obj, 0)->subindex);
   EXPECT_EQ (NULL, co_entry_find (&net, obj, 1)); // Should fail
}

TEST_F (OdTest, ObjReduce)
{
   const co_obj_t * obj = find_obj (0x1018);

   int sum = 4 + 1 + 2 + 3 + 4; // sum of index 0 to 4
   EXPECT_EQ (sum, co_obj_reduce (&net, obj, obj_sum, 0, 0, 4));
}

TEST_F (OdTest, ReadArray)
{
   const co_obj_t * obj = find_obj (0x2000);
   uint64_t value;

   co_od_get_value (&net, obj, &obj->entries[1], 1, &value);
   EXPECT_EQ (1u, value);

   co_od_get_value (&net, obj, &obj->entries[1], 2, &value);
   EXPECT_EQ (2u, value);

   co_od_get_value (&net, obj, &obj->entries[1], 8, &value);
   EXPECT_EQ (8u, value);
}

TEST_F (OdTest, WriteArray)
{
   const co_obj_t * obj = find_obj (0x2000);

   co_od_set_value (&net, obj, &obj->entries[1], 1, 10);
   co_od_set_value (&net, obj, &obj->entries[1], 2, 20);
   co_od_set_value (&net, obj, &obj->entries[1], 8, 80);

   EXPECT_EQ (10u, arr2000[0]);
   EXPECT_EQ (20u, arr2000[1]);
   EXPECT_EQ (80u, arr2000[7]);
}

TEST_F (OdTest, ReadRecord)
{
   const co_obj_t * obj = find_obj (0x2001);
   uint64_t value;

   co_od_get_value (&net, obj, &obj->entries[0], 0, &value);
   co_od_get_value (&net, obj, &obj->entries[1], 1, &value);
   co_od_get_value (&net, obj, &obj->entries[2], 2, &value);

   EXPECT_EQ (3, cb2001_calls);
}

TEST_F (OdTest, NumberOfElementsUsingAccessFunction)
{
   const co_obj_t * obj = find_obj (0x2001);
   uint64_t value;

   co_od_get_value (&net, obj, &obj->entries[0], 0, &value);
   EXPECT_EQ (2u, value);
}

TEST_F (OdTest, ZeroOD)
{
   const co_obj_t * obj = find_obj (0x2000);

   co_od_set_value (&net, obj, &obj->entries[1], 1, 10);
   co_od_set_value (&net, obj, &obj->entries[1], 2, 20);
   co_od_set_value (&net, obj, &obj->entries[1], 8, 80);

   co_od_zero (&net, 0x2000, 0x2FFF);

   EXPECT_EQ (0u, arr2000[0]);
   EXPECT_EQ (0u, arr2000[1]);
   EXPECT_EQ (0u, arr2000[7]);
}

TEST_F (OdTest, DefaultValues)
{
   const co_obj_t * obj = find_obj (0x2000);

   co_od_set_value (&net, obj, &obj->entries[1], 1, 10);
   co_od_set_value (&net, obj, &obj->entries[1], 2, 20);
   co_od_set_value (&net, obj, &obj->entries[1], 8, 80);

   net.defaults = od_defaults;
   co_od_set_defaults (&net, 0x2000, 0x2FFF);

   EXPECT_EQ (11u, arr2000[0]);
   EXPECT_EQ (22u, arr2000[1]);
   EXPECT_EQ (88u, arr2000[7]);
}

TEST_F (OdTest, StoreThenLoadOD)
{
   const co_obj_t * obj1020 = find_obj (0x1020);
   uint32_t value;
   uint32_t result;

   // Store configuration date/time
   value  = 0x12345678;
   result = co_od1020_fn (&net, OD_EVENT_WRITE, obj1020, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   result = co_od1020_fn (&net, OD_EVENT_WRITE, obj1020, NULL, 2, &value);
   EXPECT_EQ (0u, result);

   co_od_store (&net, CO_STORE_COMM, 0x1000, 0x1FFF);
   co_od_reset (&net, CO_STORE_COMM, 0x1000, 0x1FFF);

   // Read configuration date/time
   result = co_od1020_fn (&net, OD_EVENT_READ, obj1020, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (0x12345678u, value);
   result = co_od1020_fn (&net, OD_EVENT_READ, obj1020, NULL, 2, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (0x12345678u, value);
}

TEST_F (OdTest, OD1010)
{
   const co_obj_t * obj1010 = find_obj (0x1010);
   uint32_t value;
   uint32_t result;

   // Saves on command only
   result = co_od1010_fn (&net, OD_EVENT_READ, obj1010, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (1u, value);

   // Bad command
   value  = 0;
   result = co_od1010_fn (&net, OD_EVENT_WRITE, obj1010, NULL, 1, &value);
   EXPECT_EQ (CO_SDO_ABORT_WRITE, result);

   value = 0x65766173; // "SAVE"

   // Store all parameters
   result = co_od1010_fn (&net, OD_EVENT_WRITE, obj1010, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (3u, store_open_calls);

   // Store communication parameters
   result = co_od1010_fn (&net, OD_EVENT_WRITE, obj1010, NULL, 2, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (4u, store_open_calls);

   // Store application parameters
   result = co_od1010_fn (&net, OD_EVENT_WRITE, obj1010, NULL, 3, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (5u, store_open_calls);

   // Store manufacturer parameters
   result = co_od1010_fn (&net, OD_EVENT_WRITE, obj1010, NULL, 4, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (6u, store_open_calls);
}

TEST_F (OdTest, OD1011)
{
   const co_obj_t * obj1011 = find_obj (0x1011);
   uint32_t value;
   uint32_t result;

   // Device restores parameters
   result = co_od1011_fn (&net, OD_EVENT_READ, obj1011, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (1u, value);

   // Bad command
   value  = 0;
   result = co_od1011_fn (&net, OD_EVENT_WRITE, obj1011, NULL, 1, &value);
   EXPECT_EQ (CO_SDO_ABORT_WRITE, result);

   value = 0x64616F6C; // "LOAD"

   // Restore all parameters
   result = co_od1011_fn (&net, OD_EVENT_WRITE, obj1011, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (3u, store_open_calls);

   // Restore communication parameters
   result = co_od1011_fn (&net, OD_EVENT_WRITE, obj1011, NULL, 2, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (4u, store_open_calls);

   // Restore application parameters
   result = co_od1011_fn (&net, OD_EVENT_WRITE, obj1011, NULL, 3, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (5u, store_open_calls);

   // Restore manufacturer parameters
   result = co_od1011_fn (&net, OD_EVENT_WRITE, obj1011, NULL, 4, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (6u, store_open_calls);
}

TEST_F (OdTest, Notify)
{
   const co_obj_t * obj = find_obj (0x6000);

   co_od_set_value (&net, obj, &obj->entries[0], 0, 1234);
   EXPECT_EQ (1u, cb_notify_calls);
   EXPECT_EQ (0x6000u, cb_notify_index);
   EXPECT_EQ (0u, cb_notify_subindex);
}
