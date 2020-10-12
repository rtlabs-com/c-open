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

#include "osal.h"
#include "options.h"
#include "log.h"
#include "co_log.h"
#include "co_main.h"

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>

#include <linux/can.h>
#include <linux/can/raw.h>

static void os_channel_rx (void * arg)
{
   os_channel_t * channel = arg;
   struct epoll_event ev, events[1];
   int epollfd;
   int nfds;
   int n;

   epollfd = epoll_create1 (0);
   if (epollfd == -1)
   {
      LOG_ERROR (CO_CAN_LOG, "epoll_create1 failed\n");
      return;
   }

   /* Create edge-triggered event on input */
   ev.events  = EPOLLIN | EPOLLET;
   ev.data.fd = channel->handle;
   if (epoll_ctl (epollfd, EPOLL_CTL_ADD, channel->handle, &ev) == -1)
   {
      LOG_ERROR (CO_CAN_LOG, "epoll_ctl failed\n");
      return;
   }

   for (;;)
   {
      nfds = epoll_wait (epollfd, events, 1, -1);
      if (nfds == -1)
      {
         if (errno == EINTR)
            continue;

         LOG_ERROR (CO_CAN_LOG, "epoll_wait failed\n");
         return;
      }

      for (n = 0; n < nfds; n++)
      {
         if (events[n].data.fd == channel->handle)
         {
            channel->callback (channel->arg);
         }
      }
   }
}

os_channel_t * os_channel_open (const char * name, void * callback, void * arg)
{
   os_channel_t * channel = malloc (sizeof (*channel));
   struct sockaddr_can addr;
   struct ifreq ifr;

   channel->handle = socket (PF_CAN, SOCK_RAW, CAN_RAW);
   if (channel->handle < 0)
   {
      free (channel);
      return NULL;
   }

   fcntl (channel->handle, F_SETFL, O_NONBLOCK);

   strcpy (ifr.ifr_name, name);
   ioctl (channel->handle, SIOCGIFINDEX, &ifr);

   addr.can_family  = AF_CAN;
   addr.can_ifindex = ifr.ifr_ifindex;

   LOG_DEBUG (CO_CAN_LOG, "%s at index %d\n", name, ifr.ifr_ifindex);

   if (bind (channel->handle, (struct sockaddr *)&addr, sizeof (addr)) < 0)
   {
      close (channel->handle);
      free (channel);
      return NULL;
   }

   channel->callback = callback;
   channel->arg      = arg;

   os_thread_create ("co_rx", 5, 1024, os_channel_rx, channel);
   return channel;
}

int os_channel_send (os_channel_t * channel, uint32_t id, const void * data, size_t dlc)
{
   struct can_frame frame;
   int n;

   co_msg_log ("Tx", id, data, dlc);

   frame.can_id = id & CO_ID_MASK;
   frame.can_id |= (id & CO_RTR_MASK) ? CAN_RTR_FLAG : 0;
   frame.can_id |= (id & CO_EXT_MASK) ? CAN_EFF_FLAG : 0;
   frame.can_dlc = dlc;
   memcpy (frame.data, data, dlc);

   n = write (channel->handle, &frame, sizeof (struct can_frame));
   if (n != sizeof (struct can_frame))
      return -1;

   return 0;
}

int os_channel_receive (
   os_channel_t * channel,
   uint32_t * id,
   void * data,
   size_t * dlc)
{
   struct can_frame frame;
   int n;

   n = read (channel->handle, &frame, sizeof (struct can_frame));
   if (n != sizeof (struct can_frame))
      return -1;

   *id = frame.can_id;
   *id |= (frame.can_id & CAN_RTR_FLAG) ? CO_RTR_MASK : 0;
   *id |= (frame.can_id & CAN_EFF_FLAG) ? CO_EXT_MASK : 0;
   *dlc = frame.can_dlc;
   memcpy (data, frame.data, frame.can_dlc);

   co_msg_log ("Rx", *id, data, *dlc);

   return 0;
}

int os_channel_set_bitrate (os_channel_t * channel, int bitrate)
{
   return 0;
}

int os_channel_set_filter (os_channel_t * channel, uint8_t * filter, size_t size)
{
   return 0;
}

int os_channel_bus_on (os_channel_t * channel)
{
   return 0;
}

int os_channel_bus_off (os_channel_t * channel)
{
   return 0;
}

int os_channel_get_state (os_channel_t * channel, os_channel_state_t * state)
{
   return 0;
}
