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

# Fix inclusion order of assert.h and log.h by including our
# definition before anything else. FIXME: these files should be
# renamed.
include_directories(BEFORE
  src/osal/rt-kernel/
  src
  )

target_include_directories(canopen
  PRIVATE
  src/osal/rt-kernel
  )

target_sources(canopen
  PRIVATE
  src/osal/rt-kernel/osal.c
  src/osal/rt-kernel/osal_can.c
  )

target_compile_options(canopen
  PRIVATE
  -Wall
  -Wextra
  -Werror
  -Wno-unused-parameter
  )

install (FILES
  include/co_rtk.h
  DESTINATION include
  )

target_include_directories(slave
  PRIVATE
  src/osal/rt-kernel
  )

target_include_directories(slaveinfo
  PRIVATE
  src/osal/rt-kernel
  )

if (BUILD_TESTING)
  target_sources(co_test
    PRIVATE
    ${CANOPEN_SOURCE_DIR}/src/osal/rt-kernel/osal.c
    ${CANOPEN_SOURCE_DIR}/src/osal/rt-kernel/stubs.c
    )
  target_include_directories(co_test
    PRIVATE
    src/osal/rt-kernel
    )
endif()
