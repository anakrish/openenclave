# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

set(EDL_DIR ${CMAKE_SOURCE_DIR}/include/openenclave/edl)
set(LOGGING_EDL_FILE ${EDL_DIR}/logging.edl)

add_custom_command(
  OUTPUT logging_t.h logging_args.h
  DEPENDS ${CORE_EDL_FILE} edger8r
  COMMAND edger8r --header-only --search-path ${OE_INCLUDE_DIR} --trusted
          ${LOGGING_EDL_FILE})

add_enclave_library(oelogging_enc STATIC tracee.c logging_t.h)

enclave_link_libraries(oelogging_enc oe_includes oecore)

enclave_include_directories(oelogging_enc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
