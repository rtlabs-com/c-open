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

#include "co_pdo.h"
#include "co_util.h"
#include "test_util.h"

// Test fixture

class PdoTest : public TestBase
{
protected:
   virtual void SetUp() {
      TestBase::SetUp();

      net.pdo_rx[0].cobid              = CO_COBID_INVALID | 0x201;
      net.pdo_rx[0].transmission_type  = 0xFF;
      net.pdo_rx[0].number_of_mappings = 1;
      net.pdo_rx[0].mappings[0]        = 0x70000020;
      net.pdo_rx[0].bitlength          = 32;
      net.pdo_rx[0].objs[0]            = find_obj (0x7000);
      net.pdo_rx[0].entries[0]         = find_entry (net.pdo_rx[0].objs[0], 0);

      net.pdo_tx[0].cobid              = 0x181;
      net.pdo_tx[0].transmission_type  = 0xFF;
      net.pdo_tx[0].number_of_mappings = 1;
      net.pdo_tx[0].mappings[0]        = 0x60000020;
      net.pdo_tx[0].bitlength          = 32;
      net.pdo_tx[0].objs[0]            = find_obj (0x6000);
      net.pdo_tx[0].entries[0]         = find_entry (net.pdo_tx[0].objs[0], 0);

      value7000 = 0;
   }

};

// Tests

TEST_F (PdoTest, PaddingTypes)
{
   uint8_t types[] = {
      1, 2, 3, 4, 5, 6, 7,
      16,
      18, 19, 20, 21, 22,
      24, 25, 26, 27 };

   EXPECT_FALSE (co_is_padding (0, 0));
   for (size_t i = 0; i < NELEMENTS (types); i++)
   {
      EXPECT_TRUE (co_is_padding (types[i], 0));
      EXPECT_FALSE (co_is_padding (types[i], 1));
   }
   EXPECT_FALSE (co_is_padding (0x1C, 0));
}

TEST_F (PdoTest, Pack)
{
   co_pdo_t pdo;
   uint8_t * frame = (uint8_t *)&pdo.frame;
   const co_obj_t * obj6003 = find_obj (0x6003);

   memset (&pdo, 0, sizeof(pdo));

   pdo.number_of_mappings = 2;
   pdo.mappings[0] = 0x60030110;
   pdo.mappings[1] = 0x60030208;
   pdo.entries[0] = find_entry (obj6003, 1);
   pdo.entries[1] = find_entry (obj6003, 2);;
   pdo.objs[0] = obj6003;
   pdo.objs[1] = obj6003;

   co_pdo_pack (&net, &pdo);
   EXPECT_EQ (99u, frame[2]);
}

TEST_F (PdoTest, PackLarge)
{
   co_pdo_t pdo;
   uint8_t * frame = (uint8_t *)&pdo.frame;
   const co_obj_t * obj6003 = find_obj (0x6003);

   memset (&pdo, 0, sizeof(pdo));

   pdo.number_of_mappings = 2;
   pdo.mappings[0] = 0x60030320;
   pdo.mappings[1] = 0x60030408;
   pdo.entries[0] = find_entry (obj6003, 3);
   pdo.entries[1] = find_entry (obj6003, 4);
   pdo.objs[0] = obj6003;
   pdo.objs[1] = obj6003;

   co_pdo_pack (&net, &pdo);
   EXPECT_EQ (99u, frame[4]);
}

TEST_F (PdoTest, PackSmall)
{
   co_pdo_t pdo;
   uint8_t * frame = (uint8_t *)&pdo.frame;
   const co_obj_t * obj6003 = find_obj (0x6003);

   memset (&pdo, 0, sizeof(pdo));

   pdo.number_of_mappings = 2;
   pdo.mappings[0] = 0x60030501;
   pdo.mappings[1] = 0x60030601;
   pdo.entries[0] = find_entry (obj6003, 5);
   pdo.entries[1] = find_entry (obj6003, 6);
   pdo.objs[0] = obj6003;
   pdo.objs[1] = obj6003;

   co_pdo_pack (&net, &pdo);
   EXPECT_EQ (2u, frame[0]);
}

TEST_F (PdoTest, PackWithPadding)
{
   co_pdo_t pdo;
   uint8_t * frame = (uint8_t *)&pdo.frame;
   const co_obj_t * obj6003 = find_obj (0x6003);

   memset (&pdo, 0, sizeof(pdo));

   pdo.number_of_mappings = 3;
   pdo.mappings[0] = 0x60030601;
   pdo.mappings[1] = 0x00010007;
   pdo.mappings[2] = 0x60030601;
   pdo.entries[0] = find_entry (obj6003, 6);
   pdo.entries[1] = NULL;
   pdo.entries[2] = find_entry (obj6003, 6);
   pdo.objs[0] = obj6003;
   pdo.objs[1] = NULL;
   pdo.objs[2] = obj6003;

   co_pdo_pack (&net, &pdo);
   EXPECT_EQ (1u, frame[0]);
   EXPECT_EQ (1u, frame[1]);
}

TEST_F (PdoTest, Unpack)
{
   co_pdo_t pdo;
   uint8_t * frame = (uint8_t *)&pdo.frame;
   const co_obj_t * obj6003 = find_obj (0x6003);

   memset (&pdo, 0, sizeof(pdo));

   pdo.number_of_mappings = 2;
   pdo.mappings[0] = 0x60030710;
   pdo.mappings[1] = 0x60030808;
   pdo.entries[0] = find_entry (obj6003, 7);
   pdo.entries[1] = find_entry (obj6003, 8);
   pdo.objs[0] = obj6003;
   pdo.objs[1] = obj6003;

   frame[0] = 0x12;
   frame[1] = 0x34;
   frame[2] = 0x56;

   co_pdo_unpack (&net, &pdo);

   EXPECT_EQ (0x3412u, value6003_07);
   EXPECT_EQ (0x56u, value6003_08);
}

TEST_F (PdoTest, UnpackLarge)
{
   co_pdo_t pdo;
   uint8_t * frame = (uint8_t *)&pdo.frame;
   const co_obj_t * obj6003 = find_obj (0x6003);

   memset (&pdo, 0, sizeof(pdo));

   pdo.number_of_mappings = 2;
   pdo.mappings[0] = 0x60030920;
   pdo.mappings[1] = 0x60030A08;
   pdo.entries[0] = find_entry (obj6003, 9);
   pdo.entries[1] = find_entry (obj6003, 10);
   pdo.objs[0] = obj6003;
   pdo.objs[1] = obj6003;

   frame[0] = 0x00;
   frame[1] = 0x11;
   frame[2] = 0x22;
   frame[3] = 0x33;
   frame[4] = 0x44;

   co_pdo_unpack (&net, &pdo);

   EXPECT_EQ (0x33221100u, value6003_09);
   EXPECT_EQ (0x44u, value6003_0A);
}

TEST_F (PdoTest, UnpackWithPadding)
{
   co_pdo_t pdo;
   uint8_t * frame = (uint8_t *)&pdo.frame;
   const co_obj_t * obj6003 = find_obj (0x6003);

   memset (&pdo, 0, sizeof(pdo));

   pdo.number_of_mappings = 3;
   pdo.mappings[0] = 0x60030808;
   pdo.mappings[1] = 0x00050008;
   pdo.mappings[2] = 0x60030710;
   pdo.entries[0] = find_entry (obj6003, 8);
   pdo.entries[1] = NULL;
   pdo.entries[2] = find_entry (obj6003, 7);
   pdo.objs[0] = obj6003;
   pdo.objs[1] = NULL;
   pdo.objs[2] = obj6003;

   frame[0] = 0x00;
   frame[1] = 0x11;
   frame[2] = 0x22;
   frame[3] = 0x33;

   co_pdo_unpack (&net, &pdo);

   EXPECT_EQ (0x00u, value6003_08);
   EXPECT_EQ (0x3322u, value6003_07);
}

TEST_F (PdoTest, CommParamsSet)
{
   const co_obj_t * obj1400 = find_obj (0x1400);
   const co_obj_t * obj1800 = find_obj (0x1800);
   uint32_t value;
   uint32_t result;

   // Bad COB ID
   value = 0x7F;
   result = co_od1400_fn (&net, OD_EVENT_WRITE, obj1400, NULL, 1, &value);
   EXPECT_EQ (CO_SDO_ABORT_VALUE, result);
   EXPECT_EQ (CO_COBID_INVALID | 0x201u, net.pdo_rx[0].cobid);

   // Invalid (but legal) RPDO COB ID
   value = CO_COBID_INVALID | 0x207;
   result = co_od1400_fn (&net, OD_EVENT_WRITE, obj1400, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (value, net.pdo_rx[0].cobid);

   // Transmission type
   value = 0xFF;
   result = co_od1400_fn (&net, OD_EVENT_WRITE, obj1400, NULL, 2, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (value, net.pdo_rx[0].transmission_type);

   // Bad transmission type, should fail
   value = 0xF1;
   result = co_od1400_fn (&net, OD_EVENT_WRITE, obj1400, NULL, 2, &value);
   EXPECT_EQ (CO_SDO_ABORT_VALUE, result);
   EXPECT_EQ (0xFF, net.pdo_rx[0].transmission_type);

   // Inhibit time
   value = 1000;
   result = co_od1400_fn (&net, OD_EVENT_WRITE, obj1400, NULL, 3, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (value, net.pdo_rx[0].inhibit_time);

   // Bad subindex
   value = 1;
   result = co_od1400_fn (&net, OD_EVENT_WRITE, obj1400, NULL, 4, &value);
   EXPECT_EQ (CO_SDO_ABORT_BAD_SUBINDEX, result);

   // Event timer
   value = 1000;
   result = co_od1400_fn (&net, OD_EVENT_WRITE, obj1400, NULL, 5, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (value, net.pdo_rx[0].event_timer);

   // Activate COB ID
   value = 0x207;
   result = co_od1400_fn (&net, OD_EVENT_WRITE, obj1400, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (value, net.pdo_rx[0].cobid);

   // Modify inhibit time of active PDO, should fail
   value = 2000;
   result = co_od1400_fn (&net, OD_EVENT_WRITE, obj1400, NULL, 3, &value);
   EXPECT_EQ (CO_SDO_ABORT_VALUE, result);
   EXPECT_EQ (1000u, net.pdo_rx[0].inhibit_time);

   // Invalid TPDO COB ID
   value = CO_COBID_INVALID | 0x187;
   result = co_od1800_fn (&net, OD_EVENT_WRITE, obj1800, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (value, net.pdo_tx[0].cobid);

   // Sync start
   value = 5;
   result = co_od1800_fn (&net, OD_EVENT_WRITE, obj1800, NULL, 6, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (value, net.pdo_tx[0].sync_start);
}

TEST_F (PdoTest, CommParamsGet)
{
   const co_obj_t * obj1400 = find_obj (0x1400);
   uint32_t value;
   uint32_t result;

   net.pdo_rx[0].cobid             = 0x207;
   net.pdo_rx[0].transmission_type = 0xFF;
   net.pdo_rx[0].inhibit_time      = 1000;
   net.pdo_rx[0].event_timer       = 1000;
   net.pdo_rx[0].sync_start        = 5;

   // COB ID
   result = co_od1400_fn (&net, OD_EVENT_READ, obj1400, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (0x207u, value);

   // Transmission type
   result = co_od1400_fn (&net, OD_EVENT_READ, obj1400, NULL, 2, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (0xFFu, value);

   // Inhibit time
   result = co_od1400_fn (&net, OD_EVENT_READ, obj1400, NULL, 3, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (1000u, value);

   // Bad subindex
   result = co_od1400_fn (&net, OD_EVENT_READ, obj1400, NULL, 4, &value);
   EXPECT_EQ (CO_SDO_ABORT_BAD_SUBINDEX, result);

   // Event timer
   result = co_od1400_fn (&net, OD_EVENT_READ, obj1400, NULL, 5, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (1000u, value);

   // Sync start
   result = co_od1400_fn (&net, OD_EVENT_READ, obj1400, NULL, 6, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (5u, value);
}

TEST_F (PdoTest, CobId)
{
   const co_obj_t * obj1800 = find_obj (0x1800);
   uint32_t value;
   uint32_t result;

   // Invalidate (disable) cob-id, should succeed
   value = CO_COBID_INVALID | 0x181;
   result = co_od1800_fn (&net, OD_EVENT_WRITE, obj1800, NULL, 1, &value);
   EXPECT_EQ (0u, result);

   // Activate cob-id, should succeed
   value = 0x181;
   result = co_od1800_fn (&net, OD_EVENT_WRITE, obj1800, NULL, 1, &value);
   EXPECT_EQ (0u, result);

   // Modify active cob-id, should fail
   value = 0x191;
   result = co_od1800_fn (&net, OD_EVENT_WRITE, obj1800, NULL, 1, &value);
   EXPECT_EQ (CO_SDO_ABORT_VALUE, result);
}

TEST_F (PdoTest, AddMapping)
{
   const co_obj_t * obj1A00 = find_obj (0x1A00);
   uint32_t mapping = 0x60000020;
   uint32_t result;

   mock_co_obj_find_result = find_obj (0x6000);
   mock_co_entry_find_result = find_entry (mock_co_obj_find_result, 0);

   net.pdo_tx[0].cobid = CO_COBID_INVALID | 0x181;
   net.pdo_tx[0].number_of_mappings = 0;

   // Modify mapping of invalid (inactive) cob-id, should succeed
   result = co_od1A00_fn (&net, OD_EVENT_WRITE, obj1A00, NULL, 1, &mapping);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (mapping, net.pdo_tx[0].mappings[0]);
   EXPECT_EQ (mock_co_entry_find_result, net.pdo_tx[0].entries[0]);

   // Modify mapping of valid (active) cob-id, should fail
   net.pdo_tx[0].cobid = 0x181;
   result = co_od1A00_fn (&net, OD_EVENT_WRITE, obj1A00, NULL, 1, &mapping);
   EXPECT_EQ (CO_SDO_ABORT_GENERAL, result);

   // Bad bitlength, should fail
   net.pdo_tx[0].cobid = CO_COBID_INVALID | 0x181;
   mapping = 0x60000010;
   result = co_od1A00_fn (&net, OD_EVENT_WRITE, obj1A00, NULL, 1, &mapping);
   EXPECT_EQ (CO_SDO_ABORT_UNMAPPABLE, result);

   // Not mappable, should fail
   net.pdo_tx[0].cobid = CO_COBID_INVALID | 0x181;
   mapping = 0x10050020;
   mock_co_obj_find_result = find_obj (0x1005);
   mock_co_entry_find_result = find_entry (mock_co_obj_find_result, 0);
   result = co_od1A00_fn (&net, OD_EVENT_WRITE, obj1A00, NULL, 1, &mapping);
   EXPECT_EQ (CO_SDO_ABORT_UNMAPPABLE, result);
}

TEST_F (PdoTest, MappingGet)
{
   const co_obj_t * obj1A00 = find_obj (0x1A00);
   uint32_t value;
   uint32_t result;

   net.pdo_tx[0].cobid = CO_COBID_INVALID | 0x181;

   // Read bad subindex
   result = co_od1A00_fn (&net, OD_EVENT_READ, obj1A00, NULL, MAX_PDO_ENTRIES + 1, &value);
   EXPECT_EQ (CO_SDO_ABORT_BAD_SUBINDEX, result);

   // Write bad subindex
   result = co_od1A00_fn (&net, OD_EVENT_WRITE, obj1A00, NULL, MAX_PDO_ENTRIES + 1, &value);
   EXPECT_EQ (CO_SDO_ABORT_BAD_SUBINDEX, result);

   // Read number of mappings
   result = co_od1A00_fn (&net, OD_EVENT_READ, obj1A00, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (net.pdo_tx[0].number_of_mappings, value);

   // Read first mapping
   result = co_od1A00_fn (&net, OD_EVENT_READ, obj1A00, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (net.pdo_tx[0].mappings[0], value);

}

TEST_F (PdoTest, SyncWindowGetSet)
{
   uint32_t value;
   uint32_t result;

   value = 400;
   result = co_od1007_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);

   value = 0;
   result = co_od1007_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (400u, value);
}

TEST_F (PdoTest, Sync)
{
   uint8_t counter = 0;

   net.state = STATE_OP;

   // cyclic, every sync
   net.pdo_tx[0].transmission_type  = 1;

   co_pdo_sync (&net, &counter, sizeof(counter));
   co_pdo_sync (&net, &counter, sizeof(counter));
   EXPECT_EQ (2u, mock_os_channel_send_calls);
}

TEST_F (PdoTest, Sync3)
{
   uint8_t counter = 0;

   net.state = STATE_OP;

   // cyclic, every 3rd sync
   net.pdo_tx[0].transmission_type = 3;

   co_pdo_sync (&net, &counter, sizeof(counter));
   co_pdo_sync (&net, &counter, sizeof(counter));
   EXPECT_EQ (0u, mock_os_channel_send_calls);
   co_pdo_sync (&net, &counter, sizeof(counter));
   EXPECT_EQ (1u, mock_os_channel_send_calls);

   co_pdo_sync (&net, &counter, sizeof(counter));
   co_pdo_sync (&net, &counter, sizeof(counter));
   EXPECT_EQ (1u, mock_os_channel_send_calls);
   co_pdo_sync (&net, &counter, sizeof(counter));
   EXPECT_EQ (2u, mock_os_channel_send_calls);
}

TEST_F (PdoTest, SyncStart)
{
   uint8_t counter = 0;

   net.state = STATE_OP;

   // cyclic, every 2nd sync, sync_start 2
   net.sync.overflow = 3;
   net.pdo_tx[0].transmission_type = 2;
   net.pdo_tx[0].sync_start = 2;
   net.pdo_tx[0].sync_wait = 1;

   // Should ignore, sync not matched yet
   co_pdo_sync (&net, &counter, sizeof(counter));
   co_pdo_sync (&net, &counter, sizeof(counter));
   co_pdo_sync (&net, &counter, sizeof(counter));
   EXPECT_EQ (0u, mock_os_channel_send_calls);

   // First matching sync. Counter is ignored after match.
   counter = 2;
   co_pdo_sync (&net, &counter, sizeof(counter));
   co_pdo_sync (&net, &counter, sizeof(counter));
   EXPECT_EQ (1u, mock_os_channel_send_calls);

   co_pdo_sync (&net, &counter, sizeof(counter));
   co_pdo_sync (&net, &counter, sizeof(counter));
   EXPECT_EQ (2u, mock_os_channel_send_calls);
}

TEST_F (PdoTest, RxTooShort)
{
   uint8_t pdo[][3] = {
      { 0x12, 0x34, 0x56 },
   };

   net.state = STATE_OP;
   net.pdo_rx[0].cobid = 0x201;

   // Too short, should ignore data and generate emcy
   co_pdo_rx (&net, 0x201, pdo[0], sizeof(pdo[0]));
   EXPECT_EQ (1u, mock_co_emcy_tx_calls);
   EXPECT_EQ (0x8210, mock_co_emcy_tx_code);
   EXPECT_EQ (0u, value7000);
}

TEST_F (PdoTest, RxTooLong)
{
   uint8_t pdo[][5] = {
      { 0x12, 0x34, 0x56, 0x78, 0x9a },
   };

   net.state = STATE_OP;
   net.pdo_rx[0].cobid = 0x201;

   // Too long, should accept data and not generate emcy
   co_pdo_rx (&net, 0x201, pdo[0], sizeof(pdo[0]));
   EXPECT_EQ (0u, mock_co_emcy_tx_calls);
   EXPECT_EQ (0x78563412u, value7000);
}

TEST_F (PdoTest, RxEvent)
{
   uint8_t pdo[][4] = {
      { 0x11, 0x22, 0x33, 0x44 },
   };

   net.state = STATE_OP;
   net.pdo_rx[0].cobid = 0x201;
   net.pdo_rx[0].transmission_type = 0xFF;

   // Should update value immediately
   co_pdo_rx (&net, 0x201, pdo[0], sizeof(pdo[0]));
   EXPECT_EQ (0x44332211u, value7000);
}

TEST_F (PdoTest, RxSync)
{
   uint8_t counter = 0;
   uint8_t pdo[][4] = {
      { 0x11, 0x22, 0x33, 0x44 },
   };

   net.state = STATE_OP;
   net.pdo_rx[0].cobid = 0x201;
   net.pdo_rx[0].transmission_type = 0xF0;

   // Should not update value immediately
   co_pdo_rx (&net, 0x201, pdo[0], sizeof(pdo[0]));
   EXPECT_EQ (0u, value7000);

   // Should update after sync
   co_pdo_sync (&net, &counter, sizeof(counter));
   EXPECT_EQ (0x44332211u, value7000);
}

TEST_F (PdoTest, RxSyncWindow)
{
   uint8_t counter = 0;
   uint8_t pdo[][4] = {
      { 0x11, 0x22, 0x33, 0x44 },
      { 0x55, 0x66, 0x77, 0x88 },
   };

   net.state = STATE_OP;
   net.pdo_rx[0].cobid = 0x201;
   net.pdo_rx[0].transmission_type = 0xF0;
   net.sync_window = 100;

   // Start sync window
   co_pdo_sync (&net, &counter, sizeof(counter));

   // In sync window, should buffer value
   mock_os_get_current_time_us_result = 50;
   co_pdo_rx (&net, 0x201, pdo[0], sizeof(pdo[0]));
   EXPECT_EQ (0u, value7000);

   // Sync, should deliver value
   mock_os_get_current_time_us_result = 1000;
   co_pdo_sync (&net, &counter, sizeof(counter));
   EXPECT_EQ (0x44332211u, value7000);

   // Outside sync window, should not buffer value
   mock_os_get_current_time_us_result = 150;
   co_pdo_rx (&net, 0x201, pdo[1], sizeof(pdo[1]));
   EXPECT_EQ (0x44332211u, value7000);

   // Sync, should not deliver value
   mock_os_get_current_time_us_result = 2000;
   co_pdo_sync (&net, &counter, sizeof(counter));
   EXPECT_EQ (0x44332211u, value7000);
}

TEST_F (PdoTest, RtrEvent)
{
   uint8_t rtr = 0x00;

   net.state = STATE_OP;
   net.pdo_tx[0].transmission_type = 0xFD;

   // Should send PDO immediately
   co_pdo_rx (&net, CO_RTR_MASK | 0x181, &rtr, 1);
   EXPECT_EQ (0x1u, mock_os_channel_send_calls);
   EXPECT_EQ (0x181u, mock_os_channel_send_id);
}

TEST_F (PdoTest, RtrSync)
{
   uint8_t counter = 0;
   uint8_t rtr = 0x00;
   uint8_t expected[][8] = {
      { 0x01, 0x00, 0x00, 0x00 },
   };

   net.state = STATE_OP;
   net.pdo_tx[0].transmission_type = 0xFC;

   // Should sample PDO at sync
   value6000 = 1;
   co_pdo_sync (&net, &counter, sizeof(counter));

   // Should send buffered sample at RTR
   value6000 = 0;
   co_pdo_rx (&net, CO_RTR_MASK | 0x181, &rtr, 1);
   EXPECT_EQ (0x1u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x181, expected[0], 4));
}

TEST_F (PdoTest, TxEventTimer)
{
   net.state = STATE_OP;
   net.pdo_tx[0].transmission_type = 0xFF;
   net.pdo_tx[0].event_timer = 100;

   // Timer has not expired
   mock_os_get_current_time_us_result = 50 * 1000;
   co_pdo_timer (&net, mock_os_get_current_time_us_result);
   EXPECT_EQ (0x0u, mock_os_channel_send_calls);

   // Timer has expired
   mock_os_get_current_time_us_result = 150 * 1000;
   co_pdo_timer (&net, mock_os_get_current_time_us_result);
   EXPECT_EQ (0x1u, mock_os_channel_send_calls);
   EXPECT_EQ (0x181u, mock_os_channel_send_id);
}

TEST_F (PdoTest, TxInhibitTime)
{
   net.state = STATE_OP;
   net.pdo_tx[0].transmission_type = 0xFF;
   net.pdo_tx[0].inhibit_time = 100;

   // Timer has not expired
   mock_os_get_current_time_us_result = 50 * 100;
   co_pdo_trigger (&net);
   EXPECT_EQ (0x0u, mock_os_channel_send_calls);

   // Timer has expired
   mock_os_get_current_time_us_result = 150 * 100;
   co_pdo_trigger (&net);
   EXPECT_EQ (0x1u, mock_os_channel_send_calls);
   EXPECT_EQ (0x181u, mock_os_channel_send_id);
}

TEST_F (PdoTest, TxAcyclic)
{
   uint8_t counter = 0;

   net.state = STATE_OP;
   net.pdo_tx[0].transmission_type = 0x00;

   // Should not send PDO immediately
   co_pdo_trigger (&net);
   EXPECT_EQ (0x0u, mock_os_channel_send_calls);

   // Should send PDO after sync
   co_pdo_sync (&net, &counter, sizeof(counter));
   EXPECT_EQ (0x1u, mock_os_channel_send_calls);
   EXPECT_EQ (0x181u, mock_os_channel_send_id);
}

TEST_F (PdoTest, SparsePdo)
{
   const co_obj_t * obj1533 = find_obj (0x1533);
   const co_obj_t * obj1733 = find_obj (0x1733);
   const co_obj_t * obj1899 = find_obj (0x1899);
   const co_obj_t * obj1A99 = find_obj (0x1A99);
   uint32_t value;
   uint32_t result;

   // Check PDO number mapping
   EXPECT_EQ (0, net.pdo_rx[0].number);
   EXPECT_EQ (0x133, net.pdo_rx[1].number);

   // COB-ID should be invalid after init
   result = co_od1400_fn (&net, OD_EVENT_READ, obj1533, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (CO_COBID_INVALID, net.pdo_rx[1].cobid);

   // Check that 1533 is mapped to pdo_rx
   value = 0x202;
   result = co_od1400_fn (&net, OD_EVENT_WRITE, obj1533, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (0x202u, net.pdo_rx[1].cobid);

   // Check that 1733 is mapped to pdo_rx
   net.pdo_rx[1].mappings[1] = 0x1234;
   result = co_od1600_fn (&net, OD_EVENT_READ, obj1733, NULL, 2, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (0x1234u, value);

   // Check that 1533 is mapped to pdo_tx
   value = 0x182;
   result = co_od1800_fn (&net, OD_EVENT_WRITE, obj1899, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (0x182u, net.pdo_tx[1].cobid);

   // Check that 1733 is mapped to pdo_tx
   net.pdo_tx[1].mappings[1] = 0x1234;
   result = co_od1A00_fn (&net, OD_EVENT_READ, obj1A99, NULL, 2, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (0x1234u, value);
}
