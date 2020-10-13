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
const co_entry_t OD1001[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, 0, NULL },
};

/* Entry descriptor for Pre-defined error field object (1003h) */
const co_entry_t OD1003[] =
{
   { 0x00, OD_RW | OD_TRANSIENT, DTYPE_UNSIGNED8, 8, 0, NULL },
   { 0x01, OD_RO | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0, NULL },
};

/* Entry descriptor for COB-ID SYNC message object (1005h) */
const co_entry_t OD1005[] =
{
   { 0x00, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL },
};

/* Entry descriptor for Commmunication cycle object (1006h) */
const co_entry_t OD1006[] =
{
   { 0x00, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL },
};

/* Entry descriptor for Synchronous window length object (1007h) */
const co_entry_t OD1007[] =
{
   { 0x00, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL },
};

/* Entry descriptor for Guard time object (100Ch) */
const co_entry_t OD100C[] =
{
   { 0x00, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL },
};

/* Entry descriptor for Life time factor object (100Dh) */
const co_entry_t OD100D[] =
{
   { 0x00, OD_RW, DTYPE_UNSIGNED8, 8, 0, NULL },
};

/* Entry descriptor for Store Parameters object (1010h) */
const co_entry_t OD1010[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, 4, NULL },
   { 0x01, OD_RW | OD_ARRAY | OD_TRANSIENT, DTYPE_UNSIGNED32, 32, 0, NULL },
};

/* Entry descriptor for Restore Default Parameters object (1011h) */
const co_entry_t OD1011[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, 4, NULL },
   { 0x01, OD_RW | OD_ARRAY | OD_TRANSIENT, DTYPE_UNSIGNED32, 32, 0, NULL },
};

/* Entry descriptor for COB-ID EMCY object (1014h) */
const co_entry_t OD1014[] =
{
   { 0x00, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL },
};

/* Entry descriptor for Inhibit time EMCY object (1015h) */
const co_entry_t OD1015[] =
{
   { 0x00, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL },
};

/* Entry descriptor for Consumer heartbeat time object (1016h) */
const co_entry_t OD1016[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, MAX_HEARTBEATS, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0, NULL },
};

/* Entry descriptor for Producer heartbeat time object (1017h) */
const co_entry_t OD1017[] =
{
   { 0x00, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL },
};

/* Entry descriptor for Synchronous counter overflow value object (1019h) */
const co_entry_t OD1019[] =
{
   { 0x00, OD_RW, DTYPE_UNSIGNED8, 8, 0, NULL },
};

/* Entry descriptor for Verify Configuration object (1020h) */
const co_entry_t OD1020[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, 2, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0, NULL },
};

/* Entry descriptor for Emergency consumer object (1028h) */
const co_entry_t OD1028[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, MAX_EMCY_COBIDS, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0, NULL },
};

/* Entry descriptor for Error behavior object (1029h) */
const co_entry_t OD1029[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, 1, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED8, 8, 0, NULL },
};

/* Entry descriptor for RPDO communication parameter object (1400h - 15FFh) */
const co_entry_t OD1400[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8,  8,  5, NULL },
   { 0x01, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL },
   { 0x02, OD_RW, DTYPE_UNSIGNED8,  8,  0, NULL },
   { 0x03, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL },
   { 0x05, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL },
};

/* Entry descriptor for RPDO mapping parameter object (1600h - 17FFh) */
const co_entry_t OD1600[] =
{
   { 0x00, OD_RW, DTYPE_UNSIGNED8, 8, MAX_PDO_ENTRIES, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0, NULL },
};

/* Entry descriptor for TPDO communication parameter object (1800h - 19FFh) */
const co_entry_t OD1800[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8,  8,  6, NULL },
   { 0x01, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL },
   { 0x02, OD_RW, DTYPE_UNSIGNED8,  8,  0, NULL },
   { 0x03, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL },
   { 0x05, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL },
   { 0x06, OD_RW, DTYPE_UNSIGNED8,  8,  0, NULL },
};

/* Entry descriptor for RPDO mapping parameter object (1A00h - 1BFFh) */
const co_entry_t OD1A00[] =
{
   { 0x00, OD_RW, DTYPE_UNSIGNED8, 8, MAX_PDO_ENTRIES, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0, NULL },
};

/* Entry descriptor for NMT inhibit time value object (102Ah) */
const co_entry_t OD102A[] =
{
   { 0x00, OD_RW, DTYPE_UNSIGNED16, 16, 0, NULL },
};

/* Entry descriptor for Expected configuration date value object (1F26h) */
const co_entry_t OD1F26[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, CO_CONF_MNGR, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0x0, NULL },
};

/* Entry descriptor for Expected configuration time value object (1F27h) */
const co_entry_t OD1F27[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, CO_CONF_MNGR, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0x0, NULL },
};

/* Entry descriptor for NMT startup value object (1F80h) */
const co_entry_t OD1F80[] =
{
   { 0x00, OD_RW, DTYPE_UNSIGNED32, 32, 0, NULL },
};

/* Entry descriptor for NMT slave assignment value object (1F81h) */
const co_entry_t OD1F81[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, CO_CONF_MNGR, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0x0, NULL },
};

/* Entry descriptor for Request NMT value object (1F82h) */
const co_entry_t OD1F82[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, CO_CONF_MNGR + 1, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED8, 8, 0x0, NULL },
};

/* Entry descriptor for Request node guarding value object (1F83h) */
const co_entry_t OD1F83[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, CO_CONF_MNGR + 1, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED8, 8, 0x0, NULL },
};

/* Entry descriptor for Device type identification value object (1F84h) */
const co_entry_t OD1F84[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, CO_CONF_MNGR, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0x0, NULL },
};

/* Entry descriptor for Vendor identification value object (1F85h) */
const co_entry_t OD1F85[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, CO_CONF_MNGR, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0x0, NULL },
};

/* Entry descriptor for Product code value object (1F86h) */
const co_entry_t OD1F86[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, CO_CONF_MNGR, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0x0, NULL },
};

/* Entry descriptor for Revision number value object (1F87h) */
const co_entry_t OD1F87[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, CO_CONF_MNGR, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0x0, NULL },
};

/* Entry descriptor for Serial number value object (1F88h) */
const co_entry_t OD1F88[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, CO_CONF_MNGR, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0x0, NULL },
};

/* Entry descriptor for Boot time value object (1F89h) */
const co_entry_t OD1F89[] =
{
   { 0x00, OD_RO, DTYPE_UNSIGNED8, 8, CO_CONF_MNGR, NULL },
   { 0x01, OD_RW | OD_ARRAY, DTYPE_UNSIGNED32, 32, 0x0, NULL },
};
