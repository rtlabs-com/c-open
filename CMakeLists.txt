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

cmake_minimum_required (VERSION 3.13)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
project (CANOPEN VERSION 0.1.0)

include(CTest)
include(GenerateExportHeader)
include(CMakeDependentOption)

# Set required standard level
set (CMAKE_C_STANDARD 99)
set (CMAKE_CXX_STANDARD 11)

# Always use standard .o suffix
set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

# Options
option (BUILD_SHARED_LIBS "Build shared library" OFF)

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

# Default to release build with debug info
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif (NOT CMAKE_BUILD_TYPE)
message (STATUS "Current build type is: ${CMAKE_BUILD_TYPE}")

# Default to installing in build directory
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CANOPEN_BINARY_DIR}/install
    CACHE PATH "Default install path" FORCE)
endif()
message(STATUS "Current install path is: ${CMAKE_INSTALL_PREFIX}")

# Generate version numbers
configure_file (
  version.h.in
  ${CANOPEN_BINARY_DIR}/src/version.h
  )

# Generate config options
configure_file (
  co_options.h.in
  ${CANOPEN_BINARY_DIR}/include/co_options.h
  )
configure_file (
  options.h.in
  ${CANOPEN_BINARY_DIR}/src/options.h
  )

# Add platform-dependent targets early, so they can be configured by
# platform
add_library(canopen "")
add_executable (slave "")
add_executable (slaveinfo "")

if (BUILD_TESTING)
  add_executable(co_test "")
endif()

# Platform configuration
include(${CMAKE_SYSTEM_NAME})
message (STATUS "Building for ${CMAKE_SYSTEM_NAME}")

generate_export_header(canopen
  BASE_NAME co
  EXPORT_FILE_NAME ${CANOPEN_BINARY_DIR}/include/co_export.h
  )

target_include_directories(canopen
  PUBLIC
  $<BUILD_INTERFACE:${CANOPEN_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CANOPEN_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  ${CANOPEN_BINARY_DIR}/src
  src
  )


install (
  TARGETS canopen
  EXPORT CanOpenConfig
  DESTINATION lib
  )

install(
  EXPORT CanOpenConfig
  DESTINATION cmake
  )

install (FILES
  include/co_api.h
  include/co_obj.h
  ${CANOPEN_BINARY_DIR}/include/co_export.h
  ${CANOPEN_BINARY_DIR}/include/co_options.h
  DESTINATION include
  )

add_subdirectory (src)
add_subdirectory (util)

if (BUILD_TESTING)
  add_subdirectory (test)
  include(AddGoogleTest)
  add_gtest(co_test)
endif()

# Doxygen configuration
cmake_policy(SET CMP0057 NEW)
find_package(Doxygen)
if (DOXYGEN_FOUND)
  set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
  set(DOXYGEN_TYPEDEF_HIDES_STRUCT YES)
  set(DOXYGEN_EXTRACT_STATIC YES)
  set(DOXYGEN_STRIP_CODE_COMMENTS NO)
  set(DOXYGEN_MACRO_EXPANSION YES)
  set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
  set(DOXYGEN_PREDEFINED CO_EXPORT)
  set(DOXYGEN_EXPAND_AS_DEFINED CO_EXPORT)
  set(DOXYGEN_COLLABORATION_GRAPH NO)
  set(DOXYGEN_INCLUDE_GRAPH NO)
  set(DOXYGEN_INCLUDED_BY_GRAPH NO)
  set(DOXYGEN_RECURSIVE NO)
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
  set(DOXYGEN_GENERATE_LATEX YES)
  doxygen_add_docs(docs
    README.md
    include
    src
    )
endif (DOXYGEN_FOUND)

include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CANOPEN_SOURCE_DIR}/LICENSE.md")
set (CPACK_PACKAGE_CONTACT info.canopen@rt-labs.com)
include (CPack)
