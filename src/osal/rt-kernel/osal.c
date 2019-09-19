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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void os_log (uint8_t type, const char * fmt, ...)
{
   va_list list;

   switch(LOG_LEVEL_GET (type))
   {
   case LOG_LEVEL_DEBUG:   rprintf ("%10d [DEBUG] ", tick_get()); break;
   case LOG_LEVEL_INFO:    rprintf ("%10d [INFO ] ", tick_get()); break;
   case LOG_LEVEL_WARNING: rprintf ("%10d [WARN ] ", tick_get()); break;
   case LOG_LEVEL_ERROR:   rprintf ("%10d [ERROR] ", tick_get()); break;
   default: break;
   }

   va_start (list, fmt);
   vprintf (fmt, list);
   va_end (list);
}

void * os_malloc (size_t size)
{
   return malloc (size);
}

os_thread_t * os_thread_create (const char * name, uint32_t priority,
        size_t stacksize, void (*entry) (void * arg), void * arg)
{
   return task_spawn (name, entry, priority, stacksize, arg);
}

os_mutex_t * os_mutex_create (void)
{
   return mtx_create();
}

void os_mutex_lock (os_mutex_t * mutex)
{
   mtx_lock (mutex);
}

void os_mutex_unlock (os_mutex_t * mutex)
{
   mtx_unlock (mutex);
}

void os_mutex_destroy (os_mutex_t * mutex)
{
   mtx_destroy (mutex);
}

void os_usleep (uint32_t us)
{
   task_delay (tick_from_ms (us / 1000));
}

uint32_t os_get_current_time_us (void)
{
   return 1000 * tick_to_ms (tick_get());
}

os_sem_t * os_sem_create (size_t count)
{
   return sem_create (count);
}

bool os_sem_wait (os_sem_t * sem, uint32_t time)
{
   int tmo = 0;

   if (time != OS_WAIT_FOREVER)
   {
      tmo = sem_wait_tmo (sem, tick_from_ms (time));
   }
   else
   {
      sem_wait (sem);
   }

   return tmo;
}

void os_sem_signal (os_sem_t * sem)
{
   sem_signal (sem);
}

void os_sem_destroy (os_sem_t * sem)
{
   sem_destroy (sem);
}

os_event_t * os_event_create (void)
{
   return flags_create (0);
}

bool os_event_wait (os_event_t * event, uint32_t mask, uint32_t * value, uint32_t time)
{
   int tmo = 0;

   if (time != OS_WAIT_FOREVER)
   {
      tmo = flags_wait_any_tmo (event, mask, tick_from_ms (time), value);
   }
   else
   {
      flags_wait_any (event, mask, value);
   }

   *value = *value & mask;
   return tmo;
}

void os_event_set (os_event_t * event, uint32_t value)
{
   flags_set (event, value);
}

void os_event_clr (os_event_t * event, uint32_t value)
{
   flags_clr (event, value);
}

void os_event_destroy (os_event_t * event)
{
   flags_destroy (event);
}

os_mbox_t * os_mbox_create (size_t size)
{
   return mbox_create (size);
}

bool os_mbox_fetch (os_mbox_t * mbox, void ** msg, uint32_t time)
{
   int tmo = 0;

   if (time != OS_WAIT_FOREVER)
   {
      tmo = mbox_fetch_tmo (mbox, msg, tick_from_ms (time));
   }
   else
   {
      mbox_fetch (mbox, msg);
   }

   return tmo;
}

bool os_mbox_post (os_mbox_t * mbox, void * msg, uint32_t time)
{
   int tmo = 0;

   if (time != OS_WAIT_FOREVER)
   {
      tmo = mbox_post_tmo (mbox, msg, tick_from_ms (time));
   }
   else
   {
      mbox_post (mbox, msg);
   }

   return tmo;
}

void os_mbox_destroy (os_mbox_t * mbox)
{
   mbox_destroy (mbox);
}

os_timer_t * os_timer_create (uint32_t us, void (*fn) (os_timer_t *, void * arg),
                              void * arg, bool oneshot)
{
   return tmr_create (tick_from_ms (us / 1000), fn, arg,
                      (oneshot) ? TMR_ONCE : TMR_CYCL);
}

void os_timer_set (os_timer_t * timer, uint32_t us)
{
   tmr_set (timer, tick_from_ms (us / 1000));
}

void os_timer_start (os_timer_t * timer)
{
   tmr_start (timer);
}

void os_timer_stop (os_timer_t * timer)
{
   tmr_stop (timer);
}

void os_timer_destroy (os_timer_t * timer)
{
   tmr_destroy (timer);
}
