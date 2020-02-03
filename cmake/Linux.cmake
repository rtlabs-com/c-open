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

option (USE_SCHED_FIFO
  "Use SCHED_FIFO policy. May require extra privileges to run"
  OFF)

if (USE_SCHED_FIFO)
  add_compile_definitions(USE_SCHED_FIFO)
endif()

set(OSAL_SOURCES
  ${CANOPEN_SOURCE_DIR}/src/osal/linux/osal.c
  ${CANOPEN_SOURCE_DIR}/src/osal/linux/osal_can.c
  )
set(OSAL_INCLUDES
  ${CANOPEN_SOURCE_DIR}/src/osal/linux
  )
set(OSAL_LIBS
  "dl"
  "pthread"
  "rt"
  )

set(GOOGLE_TEST_INDIVIDUAL TRUE)

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-parameter -Werror")
set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})

set(CMAKE_C_FLAGS_COVERAGE "-fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS_COVERAGE ${CMAKE_C_FLAGS_COVERAGE})

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections")
