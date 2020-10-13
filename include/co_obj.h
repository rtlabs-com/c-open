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

/**
 * @file
 * @brief Communication objects handled by CANopen stack
 */

#ifndef CO_OBJ_H
#define CO_OBJ_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "co_api.h"

/** Entry descriptor for Error register object (1001h) */
extern const co_entry_t OD1001[];

/** Entry descriptor for Pre-defined error field object (1003h) */
extern const co_entry_t OD1003[];

/** Entry descriptor for COB-ID SYNC message object (1005h) */
extern const co_entry_t OD1005[];

/** Entry descriptor for Commmunication cycle object (1006h) */
extern const co_entry_t OD1006[];

/** Entry descriptor for Synchronous window length object (1007h) */
extern const co_entry_t OD1007[];

/** Entry descriptor for Guard time object (100Ch) */
extern const co_entry_t OD100C[];

/** Entry descriptor for Life time factor object (100Dh) */
extern const co_entry_t OD100D[];

/** Entry descriptor for Store Parameters object (1010h) */
extern const co_entry_t OD1010[];

/** Entry descriptor for Restore Default Parameters object (1011h) */
extern const co_entry_t OD1011[];

/** Entry descriptor for COB-ID EMCY object (1014h) */
extern const co_entry_t OD1014[];

/** Entry descriptor for Inhibit time EMCY object (1015h) */
extern const co_entry_t OD1015[];

/** Entry descriptor for Consumer heartbeat time object (1016h) */
extern const co_entry_t OD1016[];

/** Entry descriptor for Producer heartbeat time object (1017h) */
extern const co_entry_t OD1017[];

/** Entry descriptor for Synchronous counter overflow value object (1019h) */
extern const co_entry_t OD1019[];

/** Entry descriptor for Verify Configuration object (1020h) */
extern const co_entry_t OD1020[];

/** Entry descriptor for Emergency consumer object (1028h) */
extern const co_entry_t OD1028[];

/** Entry descriptor for Error behavior object (1029h) */
extern const co_entry_t OD1029[];

/** Entry descriptor for RPDO communication parameter object (1400h - 15FFh) */
extern const co_entry_t OD1400[];

/** Entry descriptor for RPDO mapping parameter object (1600h - 17FFh) */
extern const co_entry_t OD1600[];

/** Entry descriptor for TPDO communication parameter object (1800h - 19FFh) */
extern const co_entry_t OD1800[];

/** Entry descriptor for RPDO mapping parameter object (1A00h - 1BFFh) */
extern const co_entry_t OD1A00[];

/** Entry descriptor for NMT inhibit time value object (102Ah) */
extern const co_entry_t OD102A[];

/** Entry descriptor for Expected configuration date value object (1F26h) */
extern const co_entry_t OD1F26[];

/** Entry descriptor for Expected configuration time value object (1F27h) */
extern const co_entry_t OD1F27[];

/** Entry descriptor for NMT startup value object (1F80h) */
extern const co_entry_t OD1F80[];

/** Entry descriptor for NMT slave assignment value object (1F81h) */
extern const co_entry_t OD1F81[];

/** Entry descriptor for Request NMT value object (1F82h) */
extern const co_entry_t OD1F82[];

/** Entry descriptor for Request node guarding value object (1F83h) */
extern const co_entry_t OD1F83[];

/** Entry descriptor for Device type identification value object (1F84h) */
extern const co_entry_t OD1F84[];

/** Entry descriptor for Vendor identification value object (1F85h) */
extern const co_entry_t OD1F85[];

/** Entry descriptor for Product code value object (1F86h) */
extern const co_entry_t OD1F86[];

/** Entry descriptor for Revision number value object (1F87h) */
extern const co_entry_t OD1F87[];

/** Entry descriptor for Serial number value object (1F88h) */
extern const co_entry_t OD1F88[];;

/** Entry descriptor for Boot time value object (1F89h) */
extern const co_entry_t OD1F89[];

/**
 * Access function for Error register object (1001h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1001_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value
   );

/**
 * Access function for Pre-defined error field object (1003h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1003_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value
   );

/**
 * Access function for COB-ID SYNC message object (1005h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1005_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Commmunication cycle object (1006h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1006_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Synchronous window length object (1007h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1007_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Guard time object (100Ch)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od100C_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Life time factor object (100Dh)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od100D_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Store Parameters object (1010h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1010_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Restore Default Parameters object (1011h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1011_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for COB-ID EMCY object (1014h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1014_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value
   );

/**
 * Access function for Inhibit time EMCY object (1015h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1015_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value
   );

/**
 * Access function for Consumer heartbeat time object (1016h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1016_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value
   );

/**
 * Access function for Producer heartbeat time object (1017h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1017_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value
   );

/**
 * Access function for Synchronous counter overflow value object (1019h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1019_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Verify Configuration object (1020h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1020_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Emergency consumer object (1028h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1028_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value
   );

/**
 * Access function for Error behavior object (1029h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1029_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value
   );

/**
 * Access function for RPDO communication parameter object (1400h - 15FFh)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1400_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for RPDO mapping parameter object (1600h - 17FFh)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1600_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for TPDO communication parameter object (1800h - 19FFh)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1800_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for RPDO mapping parameter object (1A00h - 1BFFh)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1A00_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for NMT inhibit time value object (102Ah)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od102a_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Expected configuration date value object (1F26h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1f26_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Expected configuration time value object (1F27h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1f27_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for NMT startup value object (1F80h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1f80_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for NMT slave assignment value object (1F81h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1f81_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Request NMT value object (1F82h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1f82_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Request node guarding value object (1F83h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1f83_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Device type identification value object (1F84h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1f84_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Vendor identification value object (1F85h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1f85_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Product code value object (1F86h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1f86_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Revision number value object (1F87h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1f87_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Serial number value object (1F88h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1f88_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

/**
 * Access function for Boot time value object (1F89h)
 *
 * @param net           network handle
 * @param event         read/write/restore
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to read or write
 *
 * @return sdo abort code
 */
uint32_t co_od1f89_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value);

#ifdef __cplusplus
}
#endif

#endif /* CO_OBJ_H */
