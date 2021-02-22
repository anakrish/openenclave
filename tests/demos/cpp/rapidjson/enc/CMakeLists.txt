# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

set(EDL_FILE ../demo.edl)

add_custom_command(
  OUTPUT demo_t.h demo_t.c
  DEPENDS ${EDL_FILE} edger8r
  COMMAND
    edger8r --trusted ${EDL_FILE} --search-path ${PROJECT_SOURCE_DIR}/include
    ${DEFINE_OE_SGX} --search-path ${CMAKE_CURRENT_SOURCE_DIR})

# Needs rapidjson-dev
# nolehmann-json is also header only with a more C++ syntax.
execute_process(
  COMMAND apkman include-paths-cmake
  COMMAND_ERROR_IS_FATAL ANY
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE APKMAN_INCLUDE_PATHS)

add_enclave(
  TARGET
  demo2_enc
  UUID
  71b0822f-42a3-4543-a97c-ca491f76b82c
  SOURCES
  enc.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/demo_t.c)

enclave_include_directories(demo2_enc
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${APKMAN_INCLUDE_PATHS})

enclave_link_libraries(
  demo2_enc
  oelibcxx
  oelibc
  -Wl,--warn-unresolved-symbols)
