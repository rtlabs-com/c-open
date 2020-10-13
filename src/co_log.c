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

#include "co_log.h"
#include "co_main.h"

#include <stdio.h>
#include <inttypes.h>

void co_msg_log (char * prefix, uint32_t id, const uint8_t * data, size_t dlc)
{
   unsigned int ix;
   char s[80];
   char * p = s;

   *p = '\0';
   if (id & CO_RTR_MASK)
   {
      sprintf (p, " RTR %d", (int)dlc);
   }
   else
   {
      for (ix = 0; ix < dlc; ix++)
      {
         p += sprintf (p, " %02x", data[ix]);
      }
   }

   LOG_DEBUG (CO_CAN_LOG, "%s %04" PRIx32 ":%s\n", prefix, id & CO_ID_MASK, s);
}
