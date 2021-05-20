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
  src/ports/linux
  )

target_sources(canopen
  PRIVATE
  src/ports/linux/coal_can.c
  )

target_compile_options(canopen
  PRIVATE
  -Wall
  -Wextra
  -Werror
  -Wno-unused-parameter
  INTERFACE
  $<$<CONFIG:Coverage>:--coverage>
  )

target_link_libraries(canopen
  INTERFACE
  $<$<CONFIG:Coverage>:--coverage>
  )

target_include_directories(slave
  PRIVATE
  src/ports/linux
  )

target_include_directories(slaveinfo
  PRIVATE
  src/ports/linux
  )

if (BUILD_TESTING)
  set(GOOGLE_TEST_INDIVIDUAL TRUE)
  target_include_directories(co_test
    PRIVATE
    src/ports/linux
    )
endif()
