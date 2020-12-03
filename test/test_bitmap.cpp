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

#include "co_bitmap.h"
#include "test_util.h"

TEST (Bitmap, SetClearGet)
{
   uint32_t bm[4] = {0};

   co_bitmap_set (bm, 0);
   co_bitmap_set (bm, 31);
   co_bitmap_set (bm, 32);
   co_bitmap_set (bm, 64);
   co_bitmap_set (bm, 67);
   co_bitmap_set (bm, 95);
   co_bitmap_set (bm, 127);

   EXPECT_EQ (0x80000001u, bm[0]);
   EXPECT_EQ (0x00000001u, bm[1]);
   EXPECT_EQ (0x80000009u, bm[2]);
   EXPECT_EQ (0x80000000u, bm[3]);

   co_bitmap_clear (bm, 67);
   co_bitmap_clear (bm, 95);

   EXPECT_EQ (1, co_bitmap_get (bm, 0));
   EXPECT_EQ (0, co_bitmap_get (bm, 1));
   EXPECT_EQ (1, co_bitmap_get (bm, 31));
   EXPECT_EQ (1, co_bitmap_get (bm, 32));
   EXPECT_EQ (1, co_bitmap_get (bm, 64));
   EXPECT_EQ (0, co_bitmap_get (bm, 67));
   EXPECT_EQ (0, co_bitmap_get (bm, 95));
   EXPECT_EQ (0, co_bitmap_get (bm, 126));
   EXPECT_EQ (1, co_bitmap_get (bm, 127));
}

TEST (Bitmap, Next)
{
   uint32_t bm[4] = {0};

   co_bitmap_set (bm, 0);
   co_bitmap_set (bm, 31);
   co_bitmap_set (bm, 32);
   co_bitmap_set (bm, 64);
   co_bitmap_set (bm, 67);
   co_bitmap_set (bm, 95);
   co_bitmap_set (bm, 127);

   EXPECT_EQ (0, co_bitmap_next (bm, 0));
   EXPECT_EQ (31, co_bitmap_next (bm, 1));
   EXPECT_EQ (32, co_bitmap_next (bm, 32));
   EXPECT_EQ (64, co_bitmap_next (bm, 33));
   EXPECT_EQ (67, co_bitmap_next (bm, 65));
   EXPECT_EQ (95, co_bitmap_next (bm, 68));
   EXPECT_EQ (127, co_bitmap_next (bm, 96));

   co_bitmap_clear (bm, 127);

   EXPECT_EQ (0, co_bitmap_next (bm, 96));
}
