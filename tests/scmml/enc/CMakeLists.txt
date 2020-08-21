# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

set(EDL_FILE ../scmml.edl)

add_custom_command(
  OUTPUT scmml_t.h scmml_t.c
  DEPENDS ${EDL_FILE} edger8r
  COMMAND
    edger8r --trusted ${EDL_FILE} --search-path ${PROJECT_SOURCE_DIR}/include
    ${DEFINE_OE_SGX} --search-path ${CMAKE_CURRENT_SOURCE_DIR})

add_enclave(
  TARGET
  scmml_enc
  UUID
  83b741ee-1318-4134-b315-5b0a5c2f4db5
  SOURCES
  enc.c
  ${CMAKE_CURRENT_BINARY_DIR}/scmml_t.c)

enclave_include_directories(scmml_enc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
enclave_link_libraries(scmml_enc oelibc)
add_dependencies(scmml_enc sm)
