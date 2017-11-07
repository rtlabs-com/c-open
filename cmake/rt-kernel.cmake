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

set(OSAL_SOURCES
  ${CANOPEN_SOURCE_DIR}/src/osal/rt-kernel/stubs.c
  ${CANOPEN_SOURCE_DIR}/src/osal/rt-kernel/osal.c
  ${CANOPEN_SOURCE_DIR}/src/osal/rt-kernel/osal_can.c
  )
set(OSAL_INCLUDES
  ${CANOPEN_SOURCE_DIR}/src/osal/rt-kernel
  ${RTK}/include
  ${RTK}/include/kern
  ${RTK}/include/arch/${ARCH}
  ${RTK}/include/drivers
  )
