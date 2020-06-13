# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

set(EDL_FILE ../terminate.edl)

add_custom_command(
  OUTPUT terminate_u.h terminate_u.c
  DEPENDS ${EDL_FILE} edger8r
  COMMAND
    edger8r --untrusted ${EDL_FILE} --search-path ${PROJECT_SOURCE_DIR}/include
    --search-path ${PLATFORM_EDL_DIR} --search-path ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(terminate_host host.cpp terminate_u.c)

target_include_directories(terminate_host PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(terminate_host oehost)
