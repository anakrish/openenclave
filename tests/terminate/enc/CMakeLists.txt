# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

set(EDL_FILE ../terminate.edl)

add_custom_command(
  OUTPUT terminate_t.h terminate_t.c
  DEPENDS ${EDL_FILE} edger8r
  COMMAND
    edger8r --trusted ${EDL_FILE} --search-path ${PROJECT_SOURCE_DIR}/include
    --search-path ${PLATFORM_EDL_DIR} --search-path ${CMAKE_CURRENT_SOURCE_DIR})

add_enclave(
  TARGET
  terminate_enc
  UUID
  1eb308cb-8fcc-49f2-a457-dbaef1c3178c
  SOURCES
  enc.c
  ${CMAKE_CURRENT_BINARY_DIR}/terminate_t.c)

enclave_include_directories(terminate_enc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
enclave_link_libraries(terminate_enc oelibc)
