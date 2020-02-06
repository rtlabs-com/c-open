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

target_sources(slaveinfo
  PRIVATE
  slaveinfo.c
  main.c
  )

target_include_directories(slaveinfo
  PRIVATE
  ${CANOPEN_SOURCE_DIR}/src
  ${CANOPEN_BINARY_DIR}/src
  )

target_link_libraries (slaveinfo PUBLIC canopen)

install (TARGETS slaveinfo DESTINATION bin)
