#********************************************************************
#        _       _         _
#  _ __ | |_  _ | |  __ _ | |__   ___
# | '__|| __|(_)| | / _` || '_ \ / __|
# | |   | |_  _ | || (_| || |_) |\__ \
# |_|    \__|(_)|_| \__,_||_.__/ |___/
#
# http://www.rt-labs.com
# Copyright 2022 rt-labs AB, Sweden.
#
# This software is dual-licensed under GPLv3 and a commercial
# license. See the file LICENSE.md distributed with this software for
# full license information.
#********************************************************************

cmake_policy(SET CMP0057 NEW)

find_package(Doxygen)
find_package(Sphinx)

if (DOXYGEN_FOUND)

  # Doxygen configuration
  set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
  set(DOXYGEN_TYPEDEF_HIDES_STRUCT YES)
  set(DOXYGEN_EXTRACT_ALL YES)
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
  set(DOXYGEN_GENERATE_XML YES)
  set(DOXYGEN_WARN_AS_ERROR FAIL_ON_WARNINGS) # Doxygen 1.9+
  set(DOXYGEN_ALIASES
    [[req="\xrefitem req \"Requirement\" \"Requirements\" "]]
    )
  set(DOXYGEN_VERBATIM_VARS DOXYGEN_ALIASES)
  doxygen_add_docs(doxygen
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    USE_STAMP_FILE
    README.md
    include/co_api.h
    include/co_obj.h
    include/co_rtk.h
    src/co_sync.h
    )
endif()

if (SPHINX_FOUND AND DOXYGEN_FOUND)
  set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
  set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)

  # Targets for Sphinx html, spell checking and link checking
  macro(add_sphinx_builder)
    add_custom_target(${ARGV0}
      COMMAND
      ${SPHINX_EXECUTABLE} -M ${ARGV1}
      ${SPHINX_SOURCE} ${SPHINX_BUILD}
      -a -E -W --keep-going
      -Dbreathe_projects.co=${CMAKE_CURRENT_BINARY_DIR}/xml
      COMMENT "Running sphinx-build ${ARGV1}"
    )
    add_dependencies(${ARGV0} doxygen)
  endmacro()

  add_sphinx_builder(sphinx-html html)
  add_sphinx_builder(sphinx-pdf latexpdf)
  add_sphinx_builder(sphinx-spelling spelling)
  add_sphinx_builder(sphinx-linkcheck linkcheck)

  # Zip the documentation
  message("c-open revision: ${CANOPEN_GIT_REVISION}")
  add_custom_command(
    TARGET sphinx-html POST_BUILD
    COMMAND zip -FS -r copen_docs_${CANOPEN_GIT_REVISION}.zip sphinx/html/ > /dev/null
    COMMAND echo "The Sphinx HTML and ZIP files are located in ${CMAKE_CURRENT_BINARY_DIR}"
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/copen_docs_${CANOPEN_GIT_REVISION}.zip
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Zipping HTML documentation")

endif()
