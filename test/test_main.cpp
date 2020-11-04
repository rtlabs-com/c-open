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

#include "options.h"
#include <gtest/gtest.h>

extern "C" {
#pragma warning(disable : 4100)
void mock_os_usleep (uint32_t usec){};
os_thread_t * mock_os_thread_create (
   const char * name,
   int priority,
   int stacksize,
   void (*entry) (void * arg),
   void * arg)
{
   return NULL;
}

os_channel_t * mock_os_channel_open (const char * name)
{
   return NULL;
}
void mock_os_channel_send (os_channel_t * channel, const uint8_t * data, size_t dlc)
{
}
void mock_os_channel_receive (
   os_channel_t * channel,
   uint8_t * data,
   size_t dlc,
   int tmo)
{
}
void mock_os_channel_set_bitrate (os_channel_t * channel, int bitrate)
{
}
void mock_os_channel_set_filter (os_channel_t * channel, uint8_t * filter, size_t size)
{
}
void mock_os_channel_bus_on (os_channel_t * channel)
{
}
void mock_os_channel_bus_off (os_channel_t * channel)
{
}
}

TEST (alloc, string)
{
   ASSERT_STREQ ("hello", "world");
}
