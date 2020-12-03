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

#include "co_obj.h"

/* Entry descriptor for Error register object (1001h) */
const co_entry_t OD1001[] = {
   {0x00, OD_RO, DTYPE_UNSIGNED8, 8, 0, NULL},
};

/* Entry descriptor for Pre-defined error field object (1003h) */
const co_entry_t OD1003[] = {
   {0x00, OD_RW | OD_TRANSIENT, DTYPE_UNSIGNED8, 8, 0, NULL},
   {0x01, OD_RO | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0, NULL},
};

/* Entry descriptor for COB-ID SYNC message object (1005h) */
const co_entry_t OD1005[] = {
   {0x00, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL},
};

/* Entry descriptor for Commmunication cycle object (1006h) */
const co_entry_t OD1006[] = {
   {0x00, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL},
};

/* Entry descriptor for Synchronous window length object (1007h) */
const co_entry_t OD1007[] = {
   {0x00, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL},
};

/* Entry descriptor for Guard time object (100Ch) */
const co_entry_t OD100C[] = {
   {0x00, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL},
};

/* Entry descriptor for Life time factor object (100Dh) */
const co_entry_t OD100D[] = {
   {0x00, OD_RW, DTYPE_UNSIGNED8, 8, 0, NULL},
};

/* Entry descriptor for Store Parameters object (1010h) */
const co_entry_t OD1010[] = {
   {0x00, OD_RO, DTYPE_UNSIGNED8, 8, 4, NULL},
   {0x01, OD_RW | OD_ARRAY | OD_TRANSIENT, DTYPE_UNSIGNED32, 32, 0, NULL},
};

/* Entry descriptor for Restore Default Parameters object (1011h) */
const co_entry_t OD1011[] = {
   {0x00, OD_RO, DTYPE_UNSIGNED8, 8, 4, NULL},
   {0x01, OD_RW | OD_ARRAY | OD_TRANSIENT, DTYPE_UNSIGNED32, 32, 0, NULL},
};

/* Entry descriptor for COB-ID EMCY object (1014h) */
const co_entry_t OD1014[] = {
   {0x00, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL},
};

/* Entry descriptor for Inhibit time EMCY object (1015h) */
const co_entry_t OD1015[] = {
   {0x00, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL},
};

/* Entry descriptor for Consumer heartbeat time object (1016h) */
const co_entry_t OD1016[] = {
   {0x00, OD_RO, DTYPE_UNSIGNED8, 8, MAX_HEARTBEATS, NULL},
   {0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0, NULL},
};

/* Entry descriptor for Producer heartbeat time object (1017h) */
const co_entry_t OD1017[] = {
   {0x00, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL},
};

/* Entry descriptor for Synchronous counter overflow value object (1019h) */
const co_entry_t OD1019[] = {
   {0x00, OD_RW, DTYPE_UNSIGNED8, 8, 0, NULL},
};

/* Entry descriptor for Verify Configuration object (1020h) */
const co_entry_t OD1020[] = {
   {0x00, OD_RO, DTYPE_UNSIGNED8, 8, 2, NULL},
   {0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0, NULL},
};

/* Entry descriptor for Emergency consumer object (1028h) */
const co_entry_t OD1028[] = {
   {0x00, OD_RO, DTYPE_UNSIGNED8, 8, MAX_EMCY_COBIDS, NULL},
   {0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0, NULL},
};

/* Entry descriptor for Error behavior object (1029h) */
const co_entry_t OD1029[] = {
   {0x00, OD_RO, DTYPE_UNSIGNED8, 8, 1, NULL},
   {0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED8, 8, 0, NULL},
};

/* Entry descriptor for RPDO communication parameter object (1400h - 15FFh) */
const co_entry_t OD1400[] = {
   {0x00, OD_RO, DTYPE_UNSIGNED8, 8, 5, NULL},
   {0x01, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL},
   {0x02, OD_RW, DTYPE_UNSIGNED8, 8, 0, NULL},
   {0x03, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL},
   {0x05, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL},
};

/* Entry descriptor for RPDO mapping parameter object (1600h - 17FFh) */
const co_entry_t OD1600[] = {
   {0x00, OD_RW, DTYPE_UNSIGNED8, 8, MAX_PDO_ENTRIES, NULL},
   {0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0, NULL},
};

/* Entry descriptor for TPDO communication parameter object (1800h - 19FFh) */
const co_entry_t OD1800[] = {
   {0x00, OD_RO, DTYPE_UNSIGNED8, 8, 6, NULL},
   {0x01, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL},
   {0x02, OD_RW, DTYPE_UNSIGNED8, 8, 0, NULL},
   {0x03, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL},
   {0x05, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL},
   {0x06, OD_RW, DTYPE_UNSIGNED8, 8, 0, NULL},
};

/* Entry descriptor for RPDO mapping parameter object (1A00h - 1BFFh) */
const co_entry_t OD1A00[] = {
   {0x00, OD_RW, DTYPE_UNSIGNED8, 8, MAX_PDO_ENTRIES, NULL},
   {0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0, NULL},
};
