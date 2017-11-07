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

find_package(Canlib)

set(OSAL_SOURCES
  ${CANOPEN_SOURCE_DIR}/src/osal/windows/osal.c
  ${CANOPEN_SOURCE_DIR}/src/osal/windows/osal_can.c
  )
set(OSAL_INCLUDES
  ${CANOPEN_SOURCE_DIR}/src/osal/windows
  ${CANLIB_INCLUDE_DIR}
  )
set(OSAL_LIBS
  ${CANLIB_LIBRARY}
  )

set(GOOGLE_TEST_INDIVIDUAL TRUE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D_CRT_SECURE_NO_WARNINGS /wd4200")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_CRT_SECURE_NO_WARNINGS /wd4200")

# GTest wants /MT
 set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()
