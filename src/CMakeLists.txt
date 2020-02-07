#********************************************************************
#        _       _         _
#  _ __ | |_  _ | |  __ _ | |__   ___
# | '__|| __|(_)| | / _` || '_ \ / __|
# | |   | |_  _ | || (_| || |_) |\__ \
# |_|    \__|(_)|_| \__,_||_.__/ |___/
#
# www.rt-labs.com
# Copyright 2017 rt-labs AB, Sweden.
#
# This software is dual-licensed under GPLv3 and a commercial
# license. See the file LICENSE.md distributed with this software for
# full license information.
#*******************************************************************/

# NOTE: add headers to make them show up in an IDE
target_sources (canopen PRIVATE
  ${CANOPEN_SOURCE_DIR}/include/co_api.h
  ${CANOPEN_SOURCE_DIR}/include/co_obj.h
  co_main.c
  co_sdo_server.c
  co_sdo_client.c
  co_sdo.h
  co_pdo.c
  co_pdo.h
  co_sync.c
  co_sync.h
  co_od.c
  co_od.h
  co_heartbeat.c
  co_heartbeat.h
  co_nmt.c
  co_nmt.h
  co_emcy.c
  co_emcy.h
  co_lss.c
  co_lss.h
  co_bitmap.c
  co_bitmap.h
  co_log.c
  co_log.h
  co_node_guard.c
  co_node_guard.h
  co_obj.c
  )
