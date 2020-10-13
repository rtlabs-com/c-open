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

#ifdef UNIT_TEST
#endif

#include "co_bitmap.h"
#include "osal.h"

void co_bitmap_set (uint32_t * bm, int bit)
{
   int ix     = bit / 32;
   int offset = bit % 32;

   CC_ASSERT (bit < 128);
   bm[ix] |= BIT (offset);
}

void co_bitmap_clear (uint32_t * bm, int bit)
{
   int ix     = bit / 32;
   int offset = bit % 32;

   CC_ASSERT (bit < 128);
   bm[ix] &= ~BIT (offset);
}

int co_bitmap_get (uint32_t * bm, int bit)
{
   int ix     = bit / 32;
   int offset = bit % 32;

   CC_ASSERT (bit < 128);
   return (bm[ix] & BIT (offset)) ? 1 : 0;
}

int co_bitmap_next (uint32_t * bm, int bit)
{
   int ix;
   int offset = bit % 32;

   CC_ASSERT (bit < 128);

   for (ix = bit / 32; ix < 4; ix++)
   {
      uint32_t mask = -(int)BIT (offset);
      if (bm[ix] & mask)
         return __builtin_ctz (bm[ix] & mask) + 32 * ix;
      offset = 0;
   }

   return 0;
}
