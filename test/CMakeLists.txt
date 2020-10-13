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

target_sources(co_test PRIVATE
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
  test_node_guard.cpp
  test_heartbeat.cpp
  test_osal.cpp
  test_mngr.cpp

  # Test utils
  mocks.h
  mocks.cpp
  test_util.h

  # Testrunner
  co_test.cpp
  )

# Rebuild units to be tested with UNIT_TEST flag set. This is used to
# mock external dependencies.
target_sources(co_test PRIVATE
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
  ${CANOPEN_SOURCE_DIR}/src/co_mngr.c
  )

get_target_property(CANOPEN_OPTIONS canopen COMPILE_OPTIONS)
target_compile_options(co_test PRIVATE
  -DUNIT_TEST
  -DCO_EMCY_LOG=LOG_STATE_OFF
  -DCO_NMT_LOG=LOG_STATE_OFF
  -DCO_LSS_LOG=LOG_STATE_OFF
  -DCO_NODE_GUARD_LOG=LOG_STATE_OFF
  -DCO_HEARTBEAT_LOG=LOG_STATE_OFF
  ${CANOPEN_OPTIONS}
  )

target_include_directories(co_test
  PRIVATE
  ${CANOPEN_SOURCE_DIR}/src
  ${CANOPEN_BINARY_DIR}/src
  )

# Link against canopen target to get the cmake properties for that
# target, such as include paths and libraries.
target_link_libraries(co_test
  PRIVATE
  canopen
  )
