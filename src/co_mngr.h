/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * http://www.rt-labs.com
 * Copyright 2017 rt-labs AB, Sweden.
 * See LICENSE file in the project root for full license information.
 ********************************************************************/

#ifndef CO_MNGR_H
#define CO_MNGR_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "co_api.h"
#include "co_main.h"

#if CO_CONF_MNGR > 0

#define NMT_STARTUP_NMT_MASTER         BIT (0)
#define NMT_STARTUP_START_ALL          BIT (1)
#define NMT_STARTUP_NMT_MASTER_START   BIT (2)
#define NMT_STARTUP_START_NODE         BIT (3)
#define NMT_STARTUP_RESET_ALL          BIT (4)
#define NMT_STARTUP_FLYING_MASTER      BIT (5)
#define NMT_STARTUP_STOP_ALL_NODES     BIT (6)

#define NMT_MNGR_SLAVE              BIT (0)
#define NMT_MNGR_BOOT_SLAVE         BIT (2)
#define NMT_MNGR_MANDATORY          BIT (3)
#define NMT_MNGR_RESET_COMM         BIT (4)
#define NMT_MNGR_SW_VERSION         BIT (5)
#define NMT_MNGR_SW_UPDATE          BIT (6)
#define NMT_MNGR_RESTORE            BIT (7)

#define CO_MNGR_NMT_STARTUP_SUPPORT (NMT_STARTUP_NMT_MASTER |  \
      NMT_STARTUP_START_ALL | \
      NMT_STARTUP_NMT_MASTER_START | \
      NMT_STARTUP_START_NODE | \
      NMT_STARTUP_RESET_ALL | \
      NMT_STARTUP_STOP_ALL_NODES \
      )

uint32_t co_od102a_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

uint32_t co_od1f26_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

uint32_t co_od1f27_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

uint32_t co_od1f80_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

uint32_t co_od1f81_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

uint32_t co_od1f82_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

uint32_t co_od1f83_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

uint32_t co_od1f84_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

uint32_t co_od1f85_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

uint32_t co_od1f86_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

uint32_t co_od1f87_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

uint32_t co_od1f88_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

uint32_t co_od1f89_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

#endif

#ifdef __cplusplus
}
#endif

#endif /* CO_MNGR_H */


