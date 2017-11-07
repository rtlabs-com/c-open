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

if (MSVC)
  set(OSAL_UUT ${CANOPEN_SOURCE_DIR}/src/osal/windows/osal.c)
endif()

if (MSYS)
  set(OSAL_UUT ${CANOPEN_SOURCE_DIR}/src/osal/cygwin/osal.c)
endif()

if (UNIX)
  set(OSAL_UUT ${CANOPEN_SOURCE_DIR}/src/osal/linux/osal.c)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "rt-kernel")
  set(OSAL_UUT
    ${CANOPEN_SOURCE_DIR}/src/osal/rt-kernel/osal.c
    ${CANOPEN_SOURCE_DIR}/src/osal/rt-kernel/stubs.c
    )
endif()

include(AddGoogleTest)

add_definitions(
  -DUNIT_TEST
  -DCO_EMCY_LOG=LOG_STATE_OFF
  -DCO_NMT_LOG=LOG_STATE_OFF
  -DCO_LSS_LOG=LOG_STATE_OFF
  -DCO_NODE_GUARD_LOG=LOG_STATE_OFF
  )

add_executable(co_test
  # Units to be tested
  #${CANOPEN_SOURCE_DIR}/src/co_main.c
  ${CANOPEN_SOURCE_DIR}/src/co_sdo_server.c
  ${CANOPEN_SOURCE_DIR}/src/co_sdo_client.c
  ${CANOPEN_SOURCE_DIR}/src/co_od.c
  ${CANOPEN_SOURCE_DIR}/src/co_pdo.c
  ${CANOPEN_SOURCE_DIR}/src/co_sync.c
  ${CANOPEN_SOURCE_DIR}/src/co_nmt.c
  ${CANOPEN_SOURCE_DIR}/src/co_emcy.c
  ${CANOPEN_SOURCE_DIR}/src/co_lss.c
  ${CANOPEN_SOURCE_DIR}/src/co_bitmap.c
  ${CANOPEN_SOURCE_DIR}/src/co_node_guard.c
  ${CANOPEN_SOURCE_DIR}/src/co_heartbeat.c
  ${CANOPEN_SOURCE_DIR}/src/co_obj.c
  ${OSAL_UUT}

  # Unit tests
  #test_main.cpp
  test_sdo_server.cpp
  test_sdo_client.cpp
  test_od.cpp
  test_pdo.cpp
  test_sync.cpp
  test_nmt.cpp
  test_emcy.cpp
  test_lss.cpp
  test_bitmap.cpp
  test_osal.cpp
  test_node_guard.cpp
  test_heartbeat.cpp

  # Test utils
  mocks.h
  mocks.cpp
  test_util.h

  # Testrunner
  co_test.cpp
  )

include_directories(${OSAL_INCLUDES} .)
target_link_libraries(co_test PUBLIC ${OSAL_LIBS})
add_gtest(co_test)
