# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

set(EDL_FILE ../scmml.edl)

add_custom_command(
  OUTPUT scmml_u.h scmml_u.c
  DEPENDS ${EDL_FILE} edger8r
  COMMAND
    edger8r --untrusted ${EDL_FILE} --search-path ${PROJECT_SOURCE_DIR}/include
    ${DEFINE_OE_SGX} --search-path ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(scmml_host host.c scmml_u.c)

target_include_directories(scmml_host PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(scmml_host oehost)
