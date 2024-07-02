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

cmake_minimum_required (VERSION 3.14)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/tools")
project (COPEN VERSION 1.0.0)

# Default settings if this is the main project
if (CMAKE_PROJECT_NAME STREQUAL COPEN)
  include(CTest)

  # Make option visible in ccmake, cmake-gui
  option (BUILD_SHARED_LIBS "Build shared library" OFF)
  option (COPEN_BUILD_DOCS "Build documentation" ON)

  # Default to release build with debug info
  if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
  endif (NOT CMAKE_BUILD_TYPE)

  # Default to installing in build directory
  if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${COPEN_BINARY_DIR}/install
      CACHE PATH "Default install path" FORCE)
  endif()

  message(STATUS "Current build type is: ${CMAKE_BUILD_TYPE}")
  message(STATUS "Current install path is: ${CMAKE_INSTALL_PREFIX}")
  message(STATUS "Building for ${CMAKE_SYSTEM_NAME}")
endif()

include(CMakeDependentOption)
include(AddOsal)
include(GenerateExportHeader)
include(CMakeDependentOption)
include(GetGitRevision)

# Always use standard .o suffix
set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

set(LOG_STATE_VALUES "ON;OFF")
set(LOG_LEVEL_VALUES "DEBUG;INFO;WARNING;ERROR")

set(LOG_LEVEL ERROR CACHE STRING "default log level")
set_property(CACHE LOG_LEVEL PROPERTY STRINGS ${LOG_LEVEL_VALUES})

set(CO_ALLOC_LOG ON CACHE STRING "co_alloc log")
set_property(CACHE CO_ALLOC_LOG PROPERTY STRINGS ${LOG_STATE_VALUES})

set(CO_CAN_LOG ON CACHE STRING "dump can messages")
set_property(CACHE CO_CAN_LOG PROPERTY STRINGS ${LOG_STATE_VALUES})

set(CO_SDO_LOG ON CACHE STRING "log sdo transfers")
set_property(CACHE CO_SDO_LOG PROPERTY STRINGS ${LOG_STATE_VALUES})

set(CO_OD_LOG ON CACHE STRING "log object dictionary access")
set_property(CACHE CO_OD_LOG PROPERTY STRINGS ${LOG_STATE_VALUES})

set(CO_NMT_LOG ON CACHE STRING "log nmt events")
set_property(CACHE CO_NMT_LOG PROPERTY STRINGS ${LOG_STATE_VALUES})

set(CO_EMCY_LOG ON CACHE STRING "log emcy events")
set_property(CACHE CO_EMCY_LOG PROPERTY STRINGS ${LOG_STATE_VALUES})

set(CO_HEARTBEAT_LOG ON CACHE STRING "log heartbeat events")
set_property(CACHE CO_HEARTBEAT_LOG PROPERTY STRINGS ${LOG_STATE_VALUES})

set(CO_NODE_GUARD_LOG ON CACHE STRING "log node guard events")
set_property(CACHE CO_NODE_GUARD_LOG PROPERTY STRINGS ${LOG_STATE_VALUES})

set(CO_LSS_LOG ON CACHE STRING "log lss events")
set_property(CACHE CO_LSS_LOG PROPERTY STRINGS ${LOG_STATE_VALUES})

set(MAX_NODES "16"
  CACHE STRING "max number of statically allocated nodes")

set(MAX_EMCY_COBIDS "4"
  CACHE STRING "max number of consumed emergency cobids")

set(MAX_HEARTBEATS "4"
  CACHE STRING "max number of monitored nodes")

set(MAX_PDO_ENTRIES "8"
  CACHE STRING "max number of mapped objects per pdo")

set(MAX_TX_PDO "4"
  CACHE STRING "max number of transmit pdo:s")

set(MAX_RX_PDO "4"
  CACHE STRING "max number of receive pdo:s")

set(MAX_ERRORS "4"
  CACHE STRING "max size of error list")

set(SDO_TIMEOUT "100"
  CACHE STRING "timeout in ms for ongoing SDO transfers")

set(CO_THREAD_PRIO "10"
  CACHE STRING "priority of main thread")

set(CO_THREAD_STACK_SIZE "4096"
  CACHE STRING "stack size of main thread")

cmake_dependent_option (COPEN_BUILD_TESTING
  "Build testing on ${PROJECT_NAME}" ON
  "BUILD_TESTING" OFF
)

# Generate version numbers
configure_file (
  version.h.in
  ${COPEN_BINARY_DIR}/src/version.h
  )

# Generate config options
configure_file (
  co_options.h.in
  ${COPEN_BINARY_DIR}/include/co_options.h
  )
configure_file (
  options.h.in
  ${COPEN_BINARY_DIR}/src/options.h
  )

# Add platform-dependent targets early, so they can be configured by
# platform
add_library(copen "")
add_executable (slave "")
add_executable (slaveinfo "")

if (COPEN_BUILD_TESTING)
  add_executable(co_test "")
endif()

# Platform configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/${CMAKE_SYSTEM_NAME}.cmake)

generate_export_header(copen
  BASE_NAME co
  EXPORT_FILE_NAME ${COPEN_BINARY_DIR}/include/co_export.h
  )

set_target_properties (copen slave slaveinfo
  PROPERTIES
  C_STANDARD 99
  )

target_compile_features(copen PUBLIC c_std_99)

target_include_directories(copen
  PUBLIC
  $<BUILD_INTERFACE:${COPEN_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${COPEN_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  ${COPEN_BINARY_DIR}/src
  src
  )

target_link_libraries(copen PUBLIC osal)

install (
  TARGETS copen
  EXPORT COpenConfig
  DESTINATION lib
  )

install(
  EXPORT COpenConfig
  DESTINATION cmake
  )

install (FILES
  include/co_api.h
  include/co_obj.h
  ${COPEN_BINARY_DIR}/include/co_export.h
  ${COPEN_BINARY_DIR}/include/co_options.h
  DESTINATION include
  )

add_subdirectory (src)
add_subdirectory (util)

if (COPEN_BUILD_TESTING)
  add_subdirectory (test)
  include(AddGoogleTest)
  add_gtest(co_test)
endif()

if (COPEN_BUILD_DOCS)
  add_subdirectory(docs)

  add_custom_target(codespell
    COMMAND codespell
        ${COPEN_SOURCE_DIR}/include/
        ${COPEN_SOURCE_DIR}/docs/
        ${COPEN_SOURCE_DIR}/src/
        ${COPEN_SOURCE_DIR}/test/
        --skip *_build*
    COMMENT "Running spell check on source code"
    )
endif()


include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${COPEN_SOURCE_DIR}/LICENSE.md")
set (CPACK_PACKAGE_CONTACT info.copen@rt-labs.com)
set (CPACK_PACKAGE_NAME c-open)
set (CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
include (CPack)
