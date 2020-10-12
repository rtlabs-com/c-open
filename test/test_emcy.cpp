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

#include "co_emcy.h"
#include "co_sdo.h"
#include "test_util.h"

// Test fixture

class EmcyTest : public TestBase
{
 protected:
   virtual void SetUp()
   {
      TestBase::SetUp();
      net.emcy.cobid = 0x81;
   }

   uint8_t msef[5] = {5, 4, 3, 2, 1};
};

// Tests

TEST_F (EmcyTest, MsgFormat)
{
   uint8_t expected[8] = {0x30, 0x81, 0x03, 0x05, 0x04, 0x03, 0x02, 0x01};

   net.emcy.error = CO_ERR_CURRENT;

   co_emcy_tx (&net, 0x8130, 0x1234, msef);
   EXPECT_EQ (1u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x81, expected, 8));
}

TEST_F (EmcyTest, AddError)
{
   uint32_t value;
   uint32_t result;

   // Test insertion at head of error list

   co_emcy_tx (&net, 1, 0x1234, msef);
   EXPECT_EQ (1u, cb_emcy_calls);
   EXPECT_EQ (1u, net.number_of_errors);
   EXPECT_EQ (0x12340001u, net.errors[0]);

   co_emcy_tx (&net, 2, 0, msef);
   EXPECT_EQ (2u, cb_emcy_calls);
   EXPECT_EQ (2u, net.number_of_errors);
   EXPECT_EQ (2u, net.errors[0]);
   EXPECT_EQ (0x12340001u, net.errors[1]);

   co_emcy_tx (&net, 3, 0, msef);
   EXPECT_EQ (3u, cb_emcy_calls);
   EXPECT_EQ (3u, net.number_of_errors);
   EXPECT_EQ (3u, net.errors[0]);
   EXPECT_EQ (2u, net.errors[1]);
   EXPECT_EQ (0x12340001u, net.errors[2]);

   // Read back number of errors
   value  = 99;
   result = co_od1003_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (3u, value);

   // Read back error at position 1 (latest error)
   value  = 99;
   result = co_od1003_fn (&net, OD_EVENT_READ, NULL, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (3u, value);

   // Read at bad position
   value  = 99;
   result = co_od1003_fn (&net, OD_EVENT_READ, NULL, NULL, 4, &value);
   EXPECT_EQ (CO_SDO_ABORT_NO_DATA, result);
}

TEST_F (EmcyTest, MaxErrors)
{
   // Fill error list
   for (uint16_t ix = 0; ix < MAX_ERRORS; ix++)
   {
      co_emcy_tx (&net, ix, 0, msef);
   }
   EXPECT_EQ (MAX_ERRORS, net.number_of_errors);
   EXPECT_EQ (0u, net.errors[MAX_ERRORS - 1]);

   // Overflow list
   co_emcy_tx (&net, 4242, 0, msef);
   EXPECT_EQ (MAX_ERRORS, net.number_of_errors);
   EXPECT_EQ (4242u, net.errors[0]);
   EXPECT_EQ (1u, net.errors[MAX_ERRORS - 1]);
}

TEST_F (EmcyTest, ClearErrors)
{
   uint32_t result;
   uint32_t value;

   // Fill error list
   for (uint16_t ix = 0; ix < MAX_ERRORS; ix++)
   {
      co_emcy_tx (&net, 4242, 0, msef);
   }
   EXPECT_EQ (MAX_ERRORS, net.number_of_errors);

   // Write invalid value
   value  = 99;
   result = co_od1003_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (CO_SDO_ABORT_VALUE, result);
   EXPECT_EQ (MAX_ERRORS, net.number_of_errors);

   // Clear errors
   value  = 0;
   result = co_od1003_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (0u, net.number_of_errors);

   // Read back number of errors
   value  = 99;
   result = co_od1003_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (0u, value);
}

TEST_F (EmcyTest, EmcyCobId)
{
   uint32_t result;
   uint32_t value;

   // Write invalid value
   value  = 0x601;
   result = co_od1014_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (CO_SDO_ABORT_VALUE, result);

   // Write new cobid
   value  = 0x82;
   result = co_od1014_fn (&net, OD_EVENT_WRITE, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);

   // Read back cobid
   value  = 99;
   result = co_od1014_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (0x82u, value);
}

TEST_F (EmcyTest, EmcyConsumer)
{
   const co_obj_t * obj1028 = find_obj (0x1028);

   uint8_t emcy[8] = {0x30, 0x81, 0x03, 0x05, 0x04, 0x03, 0x02, 0x01};
   uint8_t msef[5] = {0x05, 0x04, 0x03, 0x02, 0x01};
   uint32_t result;
   uint32_t value;

   net.emcy.cobids[0] = CO_COBID_INVALID;

   // Setup node 1 emergencies
   value  = 0x81;
   result = co_od1028_fn (&net, OD_EVENT_WRITE, obj1028, NULL, 1, &value);
   EXPECT_EQ (0u, result);

   // Should call cb_emcy with emergency for node 1
   co_emcy_rx (&net, 0x81, emcy, sizeof (emcy));
   EXPECT_EQ (1u, cb_emcy_calls);
   EXPECT_EQ (1u, cb_emcy_node);
   EXPECT_EQ (0x8130u, cb_emcy_code);
   EXPECT_EQ (0x03u, cb_emcy_reg);
   EXPECT_TRUE (ArraysMatch (msef, cb_emcy_msef));

   // Should not be able to change from valid to valid cobid
   value  = 0x82;
   result = co_od1028_fn (&net, OD_EVENT_WRITE, obj1028, NULL, 1, &value);
   EXPECT_EQ (CO_SDO_ABORT_VALUE, result);

   // Should be able to change from valid to invalid cobid
   value  = CO_COBID_INVALID | 0x82;
   result = co_od1028_fn (&net, OD_EVENT_WRITE, obj1028, NULL, 1, &value);
   EXPECT_EQ (0u, result);
}

TEST_F (EmcyTest, NMTErrorBehavior)
{
   const co_obj_t * obj1029 = find_obj (0x1029);
   uint32_t result;
   uint32_t value;

   net.state = STATE_INIT;
   value     = 0;
   result    = co_od1029_fn (&net, OD_EVENT_WRITE, obj1029, NULL, 1, &value);
   EXPECT_EQ (0u, result);

   result = co_od1029_fn (&net, OD_EVENT_READ, obj1029, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (0u, value);

   co_emcy_tx (&net, 1, 0x1234, msef);
   EXPECT_EQ (STATE_INIT, net.state);

   net.state = STATE_LAST;
   co_emcy_tx (&net, 1, 0x1234, msef);
   EXPECT_EQ (STATE_LAST, net.state);

   net.state = STATE_OFF;
   co_emcy_tx (&net, 1, 0x1234, msef);
   EXPECT_EQ (STATE_OFF, net.state);

   net.state = STATE_STOP;
   co_emcy_tx (&net, 1, 0x1234, msef);
   EXPECT_EQ (STATE_STOP, net.state);

   net.state = STATE_OP;
   co_emcy_tx (&net, 1, 0x1234, msef);
   EXPECT_EQ (STATE_PREOP, net.state);

   co_emcy_tx (&net, 1, 0x1234, msef);
   EXPECT_EQ (STATE_PREOP, net.state);

   value  = 2;
   result = co_od1029_fn (&net, OD_EVENT_WRITE, obj1029, NULL, 1, &value);
   EXPECT_EQ (0u, result);
   EXPECT_EQ (2, net.error_behavior);

   net.state = STATE_INIT;
   co_emcy_tx (&net, 1, 0x1234, msef);
   EXPECT_EQ (STATE_INIT, net.state);

   net.state = STATE_OFF;
   co_emcy_tx (&net, 1, 0x1234, msef);
   EXPECT_EQ (STATE_OFF, net.state);

   net.state = STATE_STOP;
   co_emcy_tx (&net, 1, 0x1234, msef);
   EXPECT_EQ (STATE_STOP, net.state);

   net.state = STATE_OP;
   co_emcy_tx (&net, 1, 0x1234, msef);
   EXPECT_EQ (STATE_STOP, net.state);

   net.state = STATE_PREOP;
   co_emcy_tx (&net, 1, 0x1234, msef);
   EXPECT_EQ (STATE_STOP, net.state);

   co_emcy_tx (&net, 1, 0x1234, msef);
   EXPECT_EQ (STATE_STOP, net.state);
}

TEST_F (EmcyTest, EmcyOverrun)
{
   uint32_t error_register;
   uint8_t expected[][8] = {
      {0x10, 0x81, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   // Set overrun error state
   mock_os_channel_get_state_state.overrun = true;

   // Should send EMCY and add communication error state
   co_emcy_handle_can_state (&net);
   co_od1001_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &error_register);
   EXPECT_EQ (CO_ERR_COMMUNICATION | CO_ERR_GENERIC, error_register);
   EXPECT_EQ (1u, net.number_of_errors); // Add error
   EXPECT_EQ (1u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x81, expected[0], 8));

   // Should do nothing, state is not changed
   co_emcy_handle_can_state (&net);
   co_od1001_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &error_register);
   EXPECT_EQ (CO_ERR_COMMUNICATION | CO_ERR_GENERIC, error_register);
   EXPECT_EQ (1u, net.number_of_errors); // No change
   EXPECT_EQ (1u, mock_os_channel_send_calls);

   // Clear overrun error state
   mock_os_channel_get_state_state.overrun = false;

   // Should clear communication error state
   co_emcy_handle_can_state (&net);
   co_od1001_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &error_register);
   EXPECT_EQ (0u, error_register);
   EXPECT_EQ (2u, net.number_of_errors); // Add "reset error" error
   EXPECT_EQ (2u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x81, expected[1], 8));
}

TEST_F (EmcyTest, EmcyErrorPassive)
{
   uint32_t error_register;
   uint8_t expected[][8] = {
      {0x20, 0x81, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   // Set error_passive error state
   mock_os_channel_get_state_state.error_passive = true;

   // Should send EMCY and add communication error state
   co_emcy_handle_can_state (&net);
   co_od1001_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &error_register);
   EXPECT_EQ (CO_ERR_COMMUNICATION | CO_ERR_GENERIC, error_register);
   EXPECT_EQ (1u, net.number_of_errors); // Add error
   EXPECT_EQ (1u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x81, expected[0], 8));

   // Should do nothing, state is not changed
   co_emcy_handle_can_state (&net);
   co_od1001_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &error_register);
   EXPECT_EQ (CO_ERR_COMMUNICATION | CO_ERR_GENERIC, error_register);
   EXPECT_EQ (1u, net.number_of_errors); // No change
   EXPECT_EQ (1u, mock_os_channel_send_calls);

   // Clear error_passive error state
   mock_os_channel_get_state_state.error_passive = false;

   // Should clear communication error state
   co_emcy_handle_can_state (&net);
   co_od1001_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &error_register);
   EXPECT_EQ (0u, error_register);
   EXPECT_EQ (2u, net.number_of_errors); // Add "reset error" error
   EXPECT_EQ (2u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x81, expected[1], 8));
}

TEST_F (EmcyTest, EmcyBusOff)
{
   uint32_t error_register;
   uint8_t expected[][8] = {
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   };

   // Set bus_off error state
   mock_os_channel_get_state_state.bus_off = true;

   // Should add communication error state but not send emergency
   co_emcy_handle_can_state (&net);
   co_od1001_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &error_register);
   EXPECT_EQ (CO_ERR_COMMUNICATION | CO_ERR_GENERIC, error_register);
   EXPECT_EQ (0u, net.number_of_errors); // No change
   EXPECT_EQ (0u, mock_os_channel_send_calls);

   // Clear bus_off error state
   mock_os_channel_get_state_state.bus_off = false;

   // Should send EMCY without error state followed by "reset error"
   co_emcy_handle_can_state (&net);
   co_od1001_fn (&net, OD_EVENT_READ, NULL, NULL, 0, &error_register);
   EXPECT_EQ (0u, error_register);
   EXPECT_EQ (2u, net.number_of_errors);
   EXPECT_EQ (2u, mock_os_channel_send_calls);
   EXPECT_TRUE (CanMatch (0x81, expected[0], 8)); // Last error
}

TEST_F (EmcyTest, EmcyInhibit)
{
   net.emcy.inhibit = 10; // 10 * 100 us

   // Should send EMCY
   mock_os_get_current_time_us_result = 10 * 100;
   co_emcy_tx (&net, 0x8130, 0x1234, msef);
   EXPECT_EQ (1u, mock_os_channel_send_calls);

   // Inhibit time active, should not send EMCY
   mock_os_get_current_time_us_result = 15 * 100;
   co_emcy_tx (&net, 0x8130, 0x1234, msef);
   EXPECT_EQ (1u, mock_os_channel_send_calls);

   // Inhibit time expired, should send EMCY
   mock_os_get_current_time_us_result = 20 * 100;
   co_emcy_tx (&net, 0x8130, 0x1234, msef);
   EXPECT_EQ (2u, mock_os_channel_send_calls);
}
