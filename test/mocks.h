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

#ifndef MOCKS_H
#define MOCKS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "co_api.h"
#include "co_main.h"

#include "osal.h"

extern uint32_t mock_os_get_current_time_us_result;
uint32_t mock_os_get_current_time_us (void);

extern unsigned int mock_os_channel_send_calls;
extern uint32_t mock_os_channel_send_id;
extern uint8_t mock_os_channel_send_data[8];
extern size_t mock_os_channel_send_dlc;
extern int mock_os_channel_send_result;
int mock_os_channel_send (os_channel_t * channel, uint32_t id,
                          const uint8_t * data, size_t dlc);

extern unsigned int mock_os_channel_receive_calls;
extern uint32_t mock_os_channel_receive_id;
extern uint8_t mock_os_channel_receive_data[8];
extern size_t mock_os_channel_receive_dlc;
extern int mock_os_channel_receive_result;
int mock_os_channel_receive (os_channel_t * channel, uint32_t * id,
                             uint8_t * data, size_t * dlc, int tmo);

extern unsigned int mock_os_channel_bus_off_calls;
int mock_os_channel_bus_off (os_channel_t * channel);

extern unsigned int mock_os_channel_bus_on_calls;
int mock_os_channel_bus_on (os_channel_t * channel);

extern unsigned int mock_os_channel_set_bitrate_calls;
extern int mock_os_channel_set_bitrate_bitrate;
int mock_os_channel_set_bitrate (os_channel_t * channel, int bitrate);

extern unsigned int mock_os_channel_set_filter_calls;
extern int mock_os_channel_set_filter_filter;
int mock_os_channel_set_filter (os_channel_t * channel, int filter);

extern unsigned int mock_os_channel_get_state_calls;
extern os_channel_state_t mock_os_channel_get_state_state;
int mock_os_channel_get_state (os_channel_t * channel, os_channel_state * state);

extern const co_obj_t * mock_co_obj_find_result;
const co_obj_t * mock_co_obj_find (co_net_t * net, uint16_t index);

extern const co_entry_t * mock_co_entry_find_result;
const co_entry_t * mock_co_entry_find (co_net_t * net, co_obj_t * obj, uint8_t subindex);

extern unsigned int mock_co_od_reset_calls;
void mock_co_od_reset (co_net_t * net);

extern unsigned int mock_co_emcy_tx_calls;
extern uint16_t mock_co_emcy_tx_code;
void mock_co_emcy_tx (co_net_t * net, uint16_t code);

extern unsigned int cb_reset_calls;
void cb_reset (void * arg);

extern unsigned int cb_nmt_calls;
void cb_nmt (void * arg, co_state_t state);

extern unsigned int cb_emcy_calls;
extern uint8_t cb_emcy_node;
extern uint16_t cb_emcy_code;
extern uint8_t cb_emcy_reg;
extern uint8_t cb_emcy_msef[5];
void cb_emcy (void * arg, uint8_t node, uint16_t code, uint8_t reg,
              uint8_t msef[5]);


extern unsigned int cb_sync_calls;
void cb_sync (void * arg);

extern unsigned int cb_notify_calls;
extern uint16_t cb_notify_index;
extern uint16_t cb_notify_subindex;
void cb_notify (void * arg, uint16_t index, uint8_t subindex);

void store_init (void);
extern unsigned int store_open_calls;
void * store_open (co_store_t store);
int store_read (void * arg, void * data, size_t size);
int store_write (void * arg, const void * data, size_t size);
int store_close (void * arg);

#ifdef __cplusplus
}
#endif

#endif /* MOCKS_H */
