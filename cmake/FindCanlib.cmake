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

# TODO: 64-bit

find_path(CANLIB_ROOT_DIR
  NAMES INC/canlib.h
  HINTS $ENV{PROGRAMFILES}/Kvaser/Canlib
  )

find_path(CANLIB_INCLUDE_DIR
  NAMES canlib.h
  HINTS ${CANLIB_ROOT_DIR}/INC
  )

find_library(CANLIB_LIBRARY
  NAMES canlib32
  HINTS ${CANLIB_ROOT_DIR}/Lib/MS
  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Canlib REQUIRED_VARS
  CANLIB_LIBRARY
  CANLIB_INCLUDE_DIR
  )

mark_as_advanced(
  CANLIB_INCLUDE_DIR
  CANLIB_LIBRARY
  )

if(CANLIB_FOUND AND NOT TARGET Canlib)
  add_library(Canlib UNKNOWN IMPORTED)
  set_target_properties(Canlib PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CANLIB_INCLUDE_DIR}"
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
    IMPORTED_LOCATION "${CANLIB_LIBRARY}"
    )
endif()
