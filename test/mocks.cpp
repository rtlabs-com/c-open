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

#include "mocks.h"

#include <gtest/gtest.h>
#include <string.h>

uint32_t mock_os_get_current_time_us_result = 0;
uint32_t mock_os_get_current_time_us (void)
{
   return mock_os_get_current_time_us_result;
}

unsigned int mock_os_channel_send_calls = 0;
uint32_t mock_os_channel_send_id;
uint8_t mock_os_channel_send_data[8];
size_t mock_os_channel_send_dlc;
int mock_os_channel_send_result;
int mock_os_channel_send (os_channel_t * channel, uint32_t id,
                                      const uint8_t * data, size_t dlc)
{
   (void)channel;
   EXPECT_LE (dlc, 8u);
   mock_os_channel_send_calls++;
   mock_os_channel_send_id = id;
   mock_os_channel_send_dlc = dlc;
   memcpy (mock_os_channel_send_data, data, dlc);
   return mock_os_channel_send_result;
}

unsigned int mock_os_channel_receive_calls = 0;
uint32_t mock_os_channel_receive_id;
uint8_t mock_os_channel_receive_data[8];
size_t mock_os_channel_receive_dlc;
int mock_os_channel_receive_result;
int mock_os_channel_receive (os_channel_t * channel, uint32_t * id,
                                        uint8_t * data, size_t * dlc, int tmo)
{
   (void)channel;
   (void)tmo;
   mock_os_channel_receive_calls++;
   *id = mock_os_channel_receive_id;
   *dlc = mock_os_channel_receive_dlc;
   memcpy (data, mock_os_channel_receive_data, *dlc);
   return mock_os_channel_receive_result;
}

unsigned int mock_os_channel_bus_off_calls = 0;
int mock_os_channel_bus_off (os_channel_t * channel)
{
   mock_os_channel_bus_off_calls++;
   return 0;
}

unsigned int mock_os_channel_bus_on_calls = 0;
int mock_os_channel_bus_on (os_channel_t * channel)
{
   mock_os_channel_bus_on_calls++;
   return 0;
}

unsigned int mock_os_channel_set_bitrate_calls = 0;
int mock_os_channel_set_bitrate_bitrate = 0;
int mock_os_channel_set_bitrate (os_channel_t * channel, int bitrate)
{
   mock_os_channel_set_bitrate_calls++;
   mock_os_channel_set_bitrate_bitrate = bitrate;
   return 0;
}

unsigned int mock_os_channel_set_filter_calls = 0;
int mock_os_channel_set_filter_filter = 0;
int mock_os_channel_set_filter (os_channel_t * channel, int filter)
{
   mock_os_channel_set_filter_calls++;
   mock_os_channel_set_filter_filter = filter;
   return 0;
}

unsigned int mock_os_channel_get_state_calls = 0;
os_channel_state_t mock_os_channel_get_state_state;
int mock_os_channel_get_state (os_channel_t * channel, os_channel_state * state)
{
   mock_os_channel_get_state_calls++;
   *state = mock_os_channel_get_state_state;
   return 0;
}


const co_obj_t * mock_co_obj_find_result;
const co_obj_t * mock_co_obj_find (co_net_t * net, uint16_t index)
{
   return mock_co_obj_find_result;
}

const co_entry_t * mock_co_entry_find_result;
const co_entry_t * mock_co_entry_find (co_net_t * net, co_obj_t * obj, uint8_t subindex)
{
   return mock_co_entry_find_result;
}

unsigned int mock_co_od_reset_calls = 0;
void mock_co_od_reset (co_net_t * net)
{
   mock_co_od_reset_calls++;
}

unsigned int mock_co_emcy_tx_calls = 0;
uint16_t mock_co_emcy_tx_code = 0;
void mock_co_emcy_tx (co_net_t * net, uint16_t code)
{
   mock_co_emcy_tx_calls++;
   mock_co_emcy_tx_code = code;
}


unsigned int cb_reset_calls;
void cb_reset (void * arg)
{
   cb_reset_calls++;
}

unsigned int cb_nmt_calls;
void cb_nmt (void * arg, co_state_t state)
{
   cb_nmt_calls++;
}

unsigned int cb_emcy_calls;
uint8_t cb_emcy_node;
uint16_t cb_emcy_code;
uint8_t cb_emcy_reg;
uint8_t cb_emcy_msef[5];
void cb_emcy (void * arg, uint8_t node, uint16_t code, uint8_t reg,
                     uint8_t msef[5])
{
   cb_emcy_calls++;
   cb_emcy_node = node;
   cb_emcy_code = code;
   cb_emcy_reg = reg;
   if (msef != NULL)
      memcpy (cb_emcy_msef, msef, sizeof(cb_emcy_msef));
}

unsigned int cb_sync_calls;
void cb_sync (void * arg)
{
   cb_sync_calls++;
}

unsigned int cb_notify_calls;
uint16_t cb_notify_index;
uint16_t cb_notify_subindex;
void cb_notify (void * arg, uint16_t index, uint8_t subindex)
{
   cb_notify_calls++;
   cb_notify_index = index;
   cb_notify_subindex = subindex;
}

uint8_t the_store[2*1024];
struct fd
{
   uint8_t * p;
} _fd;

void store_init (void)
{
   memset (the_store, 0, sizeof(the_store));
}

unsigned int store_open_calls;
void * store_open (co_store_t store)
{
   store_open_calls++;
   _fd.p = the_store;
   return &_fd;
}

int store_read (void * arg, void * data, size_t size)
{
   struct fd * fd = (struct fd *)arg;
   memcpy (data, fd->p, size);
   fd->p += size;
   return 0;
}

int store_write (void * arg, const void * data, size_t size)
{
   struct fd * fd = (struct fd *)arg;
   memcpy (fd->p, data, size);
   fd->p += size;
   return 0;
}

int store_close (void * arg)
{
   return 0;
}
