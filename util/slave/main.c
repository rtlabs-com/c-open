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

#include "slave.h"
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char * argv[])
{
   uint32_t bitrate;

   if (argc != 3)
   {
      printf ("usage: %s <canif> <bitrate>\n", argv[0]);
      return -1;
   }

   bitrate = atoi (argv[2]);

   return slave_init (argv[1], bitrate);
}
