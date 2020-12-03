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
 * @brief bitmap utility functions
 */

#ifndef CO_BITMAP_H
#define CO_BITMAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * Set a bit in a 128-bit bitmap.
 *
 * @param bm            bitmap
 * @param bit           number of bit to set (0 to 127)
 */
void co_bitmap_set (uint32_t * bm, int bit);

/**
 * Clear a bit in a 128-bit bitmap.
 *
 * @param bm            bitmap
 * @param bit           number of bit to clear (0 to 127)
 */
void co_bitmap_clear (uint32_t * bm, int bit);

/**
 * Get a bit from a 128-bit bitmap.
 *
 * @param bm            bitmap
 * @param bit           number of bit to get (0 to 127)
 */
int co_bitmap_get (uint32_t * bm, int bit);

/**
 * Return the next set bit in a 128-bit bitmap
 *
 * This function returns the next bit that is set in the bitmap, and
 * can be used to iterate over all set bits.

 * @param bm            bitmap
 * @param bit           start bit (0 to 127)
 */
int co_bitmap_next (uint32_t * bm, int bit);

#ifdef __cplusplus
}
#endif

#endif /* CO_BITMAP_H */
