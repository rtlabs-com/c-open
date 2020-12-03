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

#include "slaveinfo.h"
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char * argv[])
{
   uint8_t node;
   uint32_t bitrate;

   if (argc != 4)
   {
      printf ("usage: %s <canif> <node> <bitrate>\n", argv[0]);
      return -1;
   }

   node    = atoi (argv[2]);
   bitrate = atoi (argv[3]);

   return slaveinfo (argv[1], node, bitrate);
}
