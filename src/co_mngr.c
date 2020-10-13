/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * http://www.rt-labs.com
 * Copyright 2017 rt-labs AB, Sweden.
 * See LICENSE file in the project root for full license information.
 ********************************************************************/

#ifdef UNIT_TEST
#define co_sdo_read mock_co_sdo_read
#define co_nmt_net mock_co_nmt_net
#define co_nmt mock_co_nmt
#define os_channel_send mock_os_channel_send
#define os_channel_bus_off mock_os_channel_bus_off
#define os_channel_bus_on mock_os_channel_bus_on
#define os_channel_set_bitrate mock_os_channel_set_bitrate
#define os_channel_set_filter mock_os_channel_set_filter
#define co_od_restore mock_co_od_restore
#endif

#include "co_mngr.h"
#include <co_api.h>
#include "co_main.h"
#include "co_od.h"
#include "co_util.h"
#include "co_sdo.h"
#include "options.h"
#include "osal.h"
#include "log.h"

#include <string.h>
#include <stdbool.h>

#if CO_CONF_MNGR > 0

uint32_t co_od102a_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (event == OD_EVENT_READ)
   {
      *value = net->nmt_inhibit;
   }
   else if (event == OD_EVENT_WRITE)
   {
      net->nmt_inhibit = *value;
   }

   return 0;
}

uint32_t co_od1f26_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (event == OD_EVENT_READ)
   {
      *value = net->conf_exp_conf_date[subindex - 1];
   }
   else if (event == OD_EVENT_WRITE)
   {
      net->conf_exp_conf_date[subindex - 1] = *value;
   }
   return 0;
}

uint32_t co_od1f27_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (event == OD_EVENT_READ)
   {
      *value = net->conf_exp_conf_time[subindex - 1];
   }
   else if (event == OD_EVENT_WRITE)
   {
      net->conf_exp_conf_time[subindex - 1] = *value;
   }
   return 0;
}

uint32_t co_od1f80_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (event == OD_EVENT_READ)
   {
      *value = net->nmt_startup;
   }
   else if (event == OD_EVENT_WRITE)
   {
      if ((*value & ~CO_MNGR_NMT_STARTUP_SUPPORT) != 0)
      {
         /* Unsupported configuration */
         return CO_SDO_ABORT_VALUE;
      }
      net->nmt_startup = *value;
   }
   return 0;
}

uint32_t co_od1f81_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (event == OD_EVENT_READ)
   {
      *value = net->nmt_slave_assignment[subindex - 1];
   }
   else if (event == OD_EVENT_WRITE)
   {
      net->nmt_slave_assignment[subindex - 1] = *value;
   }
   return 0;
}

uint32_t co_od1f82_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (event == OD_EVENT_READ)
   {
      if (subindex == 128)
      {
         return CO_SDO_ABORT_ACCESS_WO;
      }
      else
      {
         *value = net->nmt_request[subindex - 1];
      }
   }
   else if (event == OD_EVENT_WRITE)
   {
      co_nmt_cmd_t nmt_cmd;

      //TODO create macro for states
      switch(*value)
      {
      case 5:     nmt_cmd = CO_NMT_OPERATIONAL;          break;
      case 4:     nmt_cmd = CO_NMT_STOPPED;              break;
      case 127:   nmt_cmd = CO_NMT_PRE_OPERATIONAL;      break;
      case 6:     nmt_cmd = CO_NMT_RESET_NODE;           break;
      case 7:     nmt_cmd = CO_NMT_RESET_COMMUNICATION;  break;
      default:
         LOG_ERROR (CO_NMT_LOG, "bad nmt request command %"PRIu32"\n", *value);
         return CO_SDO_ABORT_VALUE;
      }

      if (subindex == 128)
      {
         co_nmt_net (net, nmt_cmd , 0);
      }
      else
      {
         co_nmt_net (net, nmt_cmd , subindex);
      }
   }
   return 0;
}

static void co_od1f83_activate_node_guarding (co_net_t * net, uint8_t node)
{
   net->nmt_node_ts_resp[node - 1] = os_get_current_time_us();
   net->nmt_request_node_guard[node - 1] = 1;
}

uint32_t co_od1f83_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (event == OD_EVENT_READ)
   {
      if (subindex == 128)
      {
         return CO_SDO_ABORT_ACCESS_WO;
      }
      else
      {
         *value = net->nmt_request_node_guard[subindex - 1];
      }
   }
   else if (event == OD_EVENT_WRITE)
   {
      if (subindex == 128)
      {
         uint8_t i;

         for (i = 0; i < CO_CONF_MNGR; i++)
         {
            if (*value == 1)
            {
               co_od1f83_activate_node_guarding (net, i + 1);
            }
            else
            {
               return CO_SDO_ABORT_VALUE;
            }
         }
      }
      else if (net->node != subindex)
      {
         if (*value == 1)
         {
            co_od1f83_activate_node_guarding (net, subindex);
         }
         else
         {
            return CO_SDO_ABORT_VALUE;
         }
      }
   }
   return 0;
}

uint32_t co_od1f84_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (event == OD_EVENT_READ)
   {
      *value = net->nmt_slave_device_type[subindex - 1];
   }
   else if (event == OD_EVENT_WRITE)
   {
      net->nmt_slave_device_type[subindex - 1] = *value;
   }
   return 0;
}

uint32_t co_od1f85_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (event == OD_EVENT_READ)
   {
      *value = net->nmt_slave_vendor_id[subindex - 1];
   }
   else if (event == OD_EVENT_WRITE)
   {
      net->nmt_slave_vendor_id[subindex - 1] = *value;
   }
   return 0;
}

uint32_t co_od1f86_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (event == OD_EVENT_READ)
   {
      *value = net->nmt_slave_prod_code[subindex - 1];
   }
   else if (event == OD_EVENT_WRITE)
   {
      net->nmt_slave_prod_code[subindex - 1] = *value;
   }
   return 0;
}

uint32_t co_od1f87_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (event == OD_EVENT_READ)
   {
      *value = net->nmt_slave_rev_num[subindex - 1];
   }
   else if (event == OD_EVENT_WRITE)
   {
      net->nmt_slave_rev_num[subindex - 1] = *value;
   }
   return 0;
}

uint32_t co_od1f88_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (event == OD_EVENT_READ)
   {
      *value = net->nmt_slave_ser_num[subindex - 1];
   }
   else if (event == OD_EVENT_WRITE)
   {
      net->nmt_slave_ser_num[subindex - 1] = *value;
   }
   return 0;
}

uint32_t co_od1f89_fn (
   co_net_t * net,
   od_event_t event,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint32_t * value)
{
   if (event == OD_EVENT_READ)
   {
      *value = net->nmt_slave_boot_time[subindex - 1];
   }
   else if (event == OD_EVENT_WRITE)
   {
      net->nmt_slave_boot_time[subindex - 1] = *value;
   }
   return 0;
}

static void co_mngr_reset (co_client_t * client)
{
   co_net_t * net = client->net;
   uint8_t i;
   bool reset_all = true;
   uint32_t _max_boot_time = 0;

   for (i = 0; i < CO_CONF_MNGR; i++)
   {
      if (net->nmt_slave_boot_time[i] > _max_boot_time)
      {
         _max_boot_time = net->nmt_slave_boot_time[i];
      }
   }

   for (i = 0; (i < CO_CONF_MNGR) && reset_all; i++)
   {
      if ((net->nmt_slave_assignment[i] & NMT_MNGR_RESET_COMM) !=
            NMT_MNGR_RESET_COMM)
      {
         reset_all = false;
      }
   }

   if (reset_all)
   {
      LOG_INFO (CO_CONF_MNGR_LOG, "reset communication for all nodes\n");
      co_nmt (client, CO_NMT_RESET_COMMUNICATION, 0);
   }
   else
   {
      LOG_INFO (CO_CONF_MNGR_LOG, "reset communication for nodes individually\n");
      for (i = 0; i < 127; i++)
      {
         if (i < CO_CONF_MNGR)
         {
            if ((net->nmt_slave_assignment[i] & NMT_MNGR_RESET_COMM) ==
                  NMT_MNGR_RESET_COMM)
            {
               LOG_INFO (CO_CONF_MNGR_LOG, "reset communication for node %d\n", i + 1);
               co_nmt (client, CO_NMT_RESET_COMMUNICATION, i + 1);
            }
            else
            {
               LOG_INFO (CO_CONF_MNGR_LOG, "skipped reset communication for node %d\n", i + 1);
            }
         }
         else
         {
            LOG_INFO (CO_CONF_MNGR_LOG, "reset communication for node %d\n", i + 1);
            co_nmt (client, CO_NMT_RESET_COMMUNICATION, i + 1);
         }
      }
   }

   LOG_INFO (CO_CONF_MNGR_LOG, "Reset communication wait max boot time %"PRIu32" ms\n", _max_boot_time);
   os_usleep(_max_boot_time * 1000);
}

static void co_mngr_reset_node (co_client_t * client, uint8_t node)
{
   co_net_t * net = client->net;

   if ((net->nmt_slave_assignment[node - 1] & NMT_MNGR_RESET_COMM) ==
         NMT_MNGR_RESET_COMM)
   {
      LOG_INFO (CO_CONF_MNGR_LOG, "reset communication for node %d\n", node);
      co_nmt (client, CO_NMT_RESET_COMMUNICATION, node);
   }
   else
   {
      LOG_INFO (CO_CONF_MNGR_LOG, "skipped reset communication for node %d\n", node);
   }

   LOG_INFO (CO_CONF_MNGR_LOG, "Reset communication wait max boot time %"PRIu32" ms\n", net->nmt_slave_boot_time[node - 1]);
   os_usleep(net->nmt_slave_boot_time[node - 1] * 1000);
}

static co_mngr_status_t co_nmt_update_cfg (co_client_t * client, uint8_t node)
{
   co_net_t * net = client->net;
   co_mngr_status_t ret;

   ret = net->cb_write_dcf (net->cb_arg, node);
   if (ret != 0)
   {
      return ERROR_STATUS_J;
   }
   return ERROR_STATUS_OK;
}

static co_mngr_status_t co_nmt_chk_cfg (co_client_t * client, uint8_t node)
{
   co_net_t * net = client->net;

   if (net->node == node)
   {
      /* Ignore the managers own node during configuration/boot phase */
      return ERROR_STATUS_OK;
   }
   if ((node > 0 ) && (node <= CO_CONF_MNGR))
   {
      int ret;
      uint32_t conf_date;
      uint32_t conf_time;
      /* Enter route C */
      /* Check configuration */
      /* Configuration date */
      if ((net->conf_exp_conf_date[node - 1] == 0) &&
            (net->conf_exp_conf_time[node - 1] == 0))
      {
         /* Update configuration */
         return co_nmt_update_cfg (client, node);
      }
      else
      {
         ret = co_sdo_read (client, node, 0x1020, 1, &conf_date, sizeof(conf_date));
         if (ret != sizeof(conf_date))
         {
            return ERROR_STATUS_J;
         }

         ret = co_sdo_read (client, node, 0x1020, 2, &conf_time, sizeof(conf_time));
         if (ret != sizeof(conf_time))
         {
            return ERROR_STATUS_J;
         }

         if ((conf_date != net->conf_exp_conf_date[node - 1]) ||
               (conf_time != net->conf_exp_conf_time[node - 1]))
         {
            /* Update configuration */
            return co_nmt_update_cfg (client, node);
         }
      }
   }
   return ERROR_STATUS_J;
}

static co_mngr_status_t co_nmt_start_err_cntl (co_client_t * client, uint8_t node)
{
   co_net_t * net = client->net;
   uint8_t i;

   if ((node > 0 ) && (node <= CO_CONF_MNGR))
   {
      /* Start error control service (also from E,D)*/
      /* Consumer heartbeat ?*/
      for (i = 0; i < MAX_HEARTBEATS; i++)
      {
         if (node == net->heartbeat[i].node)
         {
            /* wait at least until heartbeat should expire once */
            os_usleep(net->heartbeat[i].time * 1000);
            if (net->heartbeat[i].is_alive)
            {
               LOG_INFO (CO_CONF_MNGR_LOG, "Error control started ok for node %d\n", node);
               return ERROR_STATUS_OK;
            }
            else
            {
               LOG_INFO (CO_CONF_MNGR_LOG, "Error control started failed for node %d\n", node);
               return ERROR_STATUS_K;
            }
         }
      }

      /* Start Node guarding ? */
      if (net->nmt_slave_assignment[node - 1] & NMT_MNGR_SLAVE)
      {
         if (((net->nmt_slave_assignment[node - 1] >> 16) & 0xFFFF) > 0)
         {
            co_od1f83_activate_node_guarding (net, node);
         }
      }
   }
   return ERROR_STATUS_OK;
}

static co_mngr_status_t co_nmt_start_slave (co_client_t * client, uint8_t node)
{
   co_net_t * net = client->net;

   /* In case the NMT slave is in NMT state Operational
    * the process boot NMT slave shall finish successfully. */
   //TODO create macro for states
   if (net->nmt_request[node - 1] == 5)
   {
      return ERROR_STATUS_OK;
   }
   else
   {
      /* The NMT master shall start the NMT slaves. */
      if ((net->nmt_startup & NMT_STARTUP_START_NODE) == NMT_STARTUP_START_NODE)
      {
         if ((net->nmt_startup & NMT_STARTUP_START_ALL))
         {
            if (net->state == 5)
            {
               co_nmt (client, CO_NMT_OPERATIONAL, 0);
            }
         }
         else
         {
            co_nmt (client, CO_NMT_OPERATIONAL, node);
         }
         return ERROR_STATUS_OK;
      }
   }
   return ERROR_STATUS_OK;
}

static co_mngr_status_t co_nmt_mngr_boot_slave (co_client_t * client, uint8_t node)
{
   co_net_t * net = client->net;

   if (net->node == node)
   {
      /* Ignore the managers own node during configuration/boot phase */
      return ERROR_STATUS_OK;
   }
   if ((node > 0 ) && (node <= CO_CONF_MNGR))
   {
      if (net->nmt_slave_assignment[node - 1] &  NMT_MNGR_SLAVE)
      {
         if (net->nmt_slave_assignment[node - 1] &  NMT_MNGR_BOOT_SLAVE)
         {
            co_mngr_status_t stat;
            uint32_t dev_type;
            uint32_t vendor_id;
            uint32_t prod_code;
            uint32_t rev_num;
            uint32_t ser_num;

            /* Get slave device type */
            int ret;

            if (net->nmt_slave_device_type[node - 1] != 0)
            {
               ret = co_sdo_read (client, node, 0x1000, 0, &dev_type, sizeof(dev_type));
               if (ret != sizeof(dev_type))
               {
                  return ERROR_STATUS_B;
               }

               if (dev_type != net->nmt_slave_device_type[node - 1])
               {
                  return ERROR_STATUS_C;
               }
            }

            /* Check Identity */
            /* Vendor ID */
            if (net->nmt_slave_vendor_id[node - 1] != 0)
            {
               ret = co_sdo_read (client, node, 0x1018, 1, &vendor_id, sizeof(vendor_id));
               if (ret != sizeof(vendor_id))
               {
                  return ERROR_STATUS_D;
               }

               if (vendor_id != net->nmt_slave_vendor_id[node - 1])
               {
                  return ERROR_STATUS_D;
               }
            }

            /* Product code */
            if (net->nmt_slave_prod_code[node - 1] != 0)
            {
               ret = co_sdo_read (client, node, 0x1018, 2, &prod_code, sizeof(prod_code));
               if (ret != sizeof(prod_code))
               {
                  return ERROR_STATUS_M;
               }

               if ((prod_code != net->nmt_slave_prod_code[node - 1]) &&
                     (net->nmt_slave_prod_code[node - 1] != 0))
               {
                  return ERROR_STATUS_M;
               }
            }

            /* Revision Number */
            if (net->nmt_slave_rev_num[node - 1] != 0)
            {
               ret = co_sdo_read (client, node, 0x1018, 3, &rev_num, sizeof(rev_num));
               if (ret != sizeof(rev_num))
               {
                  return ERROR_STATUS_N;
               }

               if (rev_num != net->nmt_slave_rev_num[node - 1])
               {
                  return ERROR_STATUS_N;
               }
            }

            /* Serial Number */
            if (net->nmt_slave_ser_num[node - 1] != 0)
            {
               ret = co_sdo_read (client, node, 0x1018, 4, &ser_num, sizeof(ser_num));
               if (ret != sizeof(ser_num))
               {
                  return ERROR_STATUS_O;
               }

               if (ser_num != net->nmt_slave_ser_num[node - 1])
               {
                  return ERROR_STATUS_O;
               }
            }

            stat = co_nmt_chk_cfg(client, node);
            if (stat != ERROR_STATUS_OK)
            {
               return stat;
            }
         }

         /* from D,E, Start error control service */
         co_nmt_start_err_cntl (client, node);

         return co_nmt_start_slave (client, node);
      }
      else
      {
         return ERROR_STATUS_A;
      }
   }
   else
   {
      return ERROR_STATUS_A;
   }
}

co_mngr_status_t co_nmt_mngr_startup (co_client_t * client)
{
   co_net_t * net = client->net;

   /* Configured as NMT master */
   if (net->nmt_startup & NMT_STARTUP_NMT_MASTER)
   {
      uint8_t i;
      co_mngr_status_t stat;
      bool boot_ok = true;

      /* NMT flying master */
      //TODO implement, now assumes won

      /* LSS required? */
      //TODO implement, skipped for now

      co_mngr_reset (client);

      /* Mandatory slaves started*/
      for (i = 0; i < CO_CONF_MNGR; i++)
      {
         stat = co_nmt_mngr_boot_slave (client, i + 1);
         if ((stat != ERROR_STATUS_OK) &&
               (net->nmt_slave_assignment[i] & NMT_MNGR_MANDATORY))
         {
            /* Halt start up procedure */
            return stat;
         }
         else
         {
            boot_ok = false;
         }
      }

      if (net->nmt_startup & NMT_STARTUP_NMT_MASTER_START)
      {
         co_nmt (client, CO_NMT_OPERATIONAL, net->node);
      }
      else
      {
         //TODO wait for application to set this node in operational
         return ERROR_STATUS_OK;
      }

      if ((net->nmt_startup & NMT_STARTUP_START_ALL) ||
            (net->nmt_startup & NMT_STARTUP_START_NODE))
      {
         if ((net->nmt_startup & NMT_STARTUP_START_ALL) &&
               (boot_ok))
         {
            /* Start remote node with node id 0 */
            co_nmt (client, CO_NMT_OPERATIONAL, 0);
         }
         else
         {
            /*  Start remote node individually */
            for (i = 0; i < CO_CONF_MNGR; i++)
            {
               if (net->nmt_slave_assignment[i] & NMT_MNGR_SLAVE)
               {
                  co_nmt (client, CO_NMT_OPERATIONAL, i + 1);
               }
            }
         }
      }
      return ERROR_STATUS_OK;
   }
   else if (net->nmt_startup & NMT_STARTUP_NMT_MASTER_START)
   {
      co_nmt (client, CO_NMT_OPERATIONAL, net->node);
      /* Enter slave mode */
   }
   return ERROR_STATUS_OK;
}

co_mngr_status_t co_nmt_mngr_startup_node (co_client_t * client, uint8_t node)
{
   co_net_t * net = client->net;

   co_mngr_status_t stat;

   co_mngr_reset_node (client, node);

   stat = co_nmt_mngr_boot_slave (client, node);
   if ((stat != ERROR_STATUS_OK) &&
         (net->nmt_slave_assignment[node - 1] & NMT_MNGR_MANDATORY))
   {
      /* Halt start up procedure */
      return stat;
   }

   return ERROR_STATUS_OK;
}
#endif
