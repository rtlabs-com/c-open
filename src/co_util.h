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

#ifndef CO_UTIL_H
#define CO_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#include "osal.h"

static inline int co_is_expired (uint32_t now, uint32_t timestamp, uint32_t timeout)
{
   uint32_t delta = now - timestamp;
   return delta >= timeout;
}

static inline bool co_validate_cob_id (uint32_t id)
{
   id = id & CO_ID_MASK;

   /* See CiA 301, chapter 7.3.5 */

   if (id == 0)
      return false;
   else if (id >= 0x001 && id <= 0x07F)
      return false;
   else if (id >= 0x101 && id < 0x180) /* Allow 0x180 to support CiA 417 */
      return false;
   else if (id >= 0x581 && id <= 0x5FF)
      return false;
   else if (id >= 0x601 && id <= 0x67F)
      return false;
   else if (id >= 0x6E0 && id <= 0x6FF)
      return false;
   else if (id >= 0x701 && id <= 0x77F)
      return false;
   else if (id >= 0x780 && id <= 0x7FF)
      return false;

   return true;
}

static inline bool co_is_padding (uint16_t index, uint8_t subindex)
{
   const uint32_t bitmask = 0x0F7D00FE;

   /* See CiA 301, chapter 7.4.7.1 */

   if (subindex != 0)
      return false;

   if (index > 0x1B)
      return false;

   return (BIT (index) & bitmask);
}

static inline uint8_t co_fetch_uint8 (const void * data)
{
   uint8_t * p = (uint8_t *)data;
   return p[0];
}

static inline uint16_t co_fetch_uint16 (const void * data)
{
   uint16_t value;

   memcpy (&value, data, sizeof (value));
   return CC_TO_LE16 (value);
}

static inline uint32_t co_fetch_uint32 (const void * data)
{
   uint32_t value;

   memcpy (&value, data, sizeof (value));
   return CC_TO_LE32 (value);
}

static inline uint64_t co_fetch_uint64 (const void * data)
{
   uint64_t value;

   memcpy (&value, data, sizeof (value));
   return CC_TO_LE64 (value);
}

static inline void * co_put_uint8 (void * data, uint8_t value)
{
   uint8_t * p = (uint8_t *)data;

   *p = value;
   return p + sizeof (value);
}

static inline void * co_put_uint16 (void * data, uint16_t value)
{
   uint8_t * p = (uint8_t *)data;

   value = CC_TO_LE16 (value);
   memcpy (data, &value, sizeof (value));
   return p + sizeof (value);
}

static inline void * co_put_uint32 (void * data, uint32_t value)
{
   uint8_t * p = (uint8_t *)data;

   value = CC_TO_LE32 (value);
   memcpy (data, &value, sizeof (value));
   return p + sizeof (value);
}

static inline void * co_put_uint64 (void * data, uint64_t value)
{
   uint8_t * p = (uint8_t *)data;

   value = CC_TO_LE64 (value);
   memcpy (data, &value, sizeof (value));
   return p + sizeof (value);
}

static inline uint8_t co_atomic_get_uint8 (const void * data)
{
   uint8_t * p = (uint8_t *)data;
   return CC_ATOMIC_GET8 (p);
}

static inline uint16_t co_atomic_get_uint16 (const void * data)
{
   uint16_t * p = (uint16_t *)data;
   CC_ASSERT (((uintptr_t)p & 0x01) == 0);
   return CC_ATOMIC_GET16 (p);
}

static inline uint32_t co_atomic_get_uint32 (const void * data)
{
   uint32_t * p = (uint32_t *)data;
   CC_ASSERT (((uintptr_t)p & 0x03) == 0);
   return CC_ATOMIC_GET32 (p);
}

static inline uint64_t co_atomic_get_uint64 (const void * data)
{
   uint64_t * p = (uint64_t *)data;
   CC_ASSERT (((uintptr_t)p & 0x07) == 0);
   return CC_ATOMIC_GET64 (p);
}

static inline void co_atomic_set_uint8 (void * data, uint8_t value)
{
   uint8_t * p = (uint8_t *)data;
   CC_ATOMIC_SET8 (p, value);
}

static inline void co_atomic_set_uint16 (void * data, uint16_t value)
{
   uint16_t * p = (uint16_t *)data;
   CC_ASSERT (((uintptr_t)p & 0x01) == 0);
   CC_ATOMIC_SET16 (p, value);
}

static inline void co_atomic_set_uint32 (void * data, uint32_t value)
{
   uint32_t * p = (uint32_t *)data;
   CC_ASSERT (((uintptr_t)p & 0x03) == 0);
   CC_ATOMIC_SET32 (p, value);
}

static inline void co_atomic_set_uint64 (void * data, uint64_t value)
{
   uint64_t * p = (uint64_t *)data;
   CC_ASSERT (((uintptr_t)p & 0x07) == 0);
   CC_ATOMIC_SET64 (p, value);
}

#ifdef __cplusplus
}
#endif

#endif /* CO_UTIL_H */
