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

target_include_directories(canopen
  PRIVATE
  src/ports/rt-kernel
  )

target_sources(canopen
  PRIVATE
  src/ports/rt-kernel/coal_can.c
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
  src/ports/rt-kernel
  )

target_include_directories(slaveinfo
  PRIVATE
  src/ports/rt-kernel
  )

if (BUILD_TESTING)
  target_include_directories(co_test
    PRIVATE
    src/ports/rt-kernel
    )
endif()
