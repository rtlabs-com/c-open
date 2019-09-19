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
#include <string.h>
#include <stdlib.h>

#include <co_api.h>

#include "osal.h"

int slaveinfo (const char * canif, uint8_t node, int bitrate)
{
   co_net_t * net;
   co_cfg_t cfg = { 0 };
   co_client_t * client;
   static const co_obj_t od_none[] = {
      { 0, OTYPE_NULL, 0, NULL, NULL },
   };

   cfg.node     = node;
   cfg.bitrate  = bitrate;
   cfg.od       = od_none;

   net = co_init (canif, &cfg);
   if (net == NULL)
   {
      printf("Init failed\n");
      return -1;
   }

   client = co_client_init (net);
   if (client == NULL)
   {
      printf("Client init failed\n");
      return -1;
   }

   co_nmt (client, CO_NMT_RESET_COMMUNICATION, 0);
   os_usleep (500*1000);        /* TODO: how to sync with slave responses? */

   node = co_node_next (client, 0);
   if (node == 0)
   {
      printf ("No nodes found\n");
      return -1;
   }

   while (node > 0)
   {
      char s[80];
      int n;

      n = co_sdo_read (client, node, 0x1008, 0, s, sizeof(s));
      if (n > 0)
      {
         s[n] = 0;
         printf ("(%d) %s\n", node, s);
      }

      n = co_sdo_read (client, node, 0x1009, 0, s, sizeof(s));
      if (n > 0)
      {
         s[n] = 0;
         printf ("(%d) %s\n", node, s);
      }

      n = co_sdo_read (client, node, 0x100a, 0, s, sizeof(s));
      if (n > 0)
      {
         s[n] = 0;
         printf ("(%d) %s\n\n", node, s);
      }

      node = co_node_next (client, node + 1);
   }

   co_nmt (client, CO_NMT_PRE_OPERATIONAL, 0);
   co_nmt (client, CO_NMT_OPERATIONAL, 0);

   co_sync (client);

   return 0;
}
