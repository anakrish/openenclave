# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

set(EDL_FILE ../demo.edl)

add_custom_command(
  OUTPUT demo_t.h demo_t.c
  DEPENDS ${EDL_FILE} edger8r
  COMMAND
    edger8r --trusted ${EDL_FILE} --search-path ${PROJECT_SOURCE_DIR}/include
    ${DEFINE_OE_SGX} --search-path ${CMAKE_CURRENT_SOURCE_DIR})

execute_process(
  COMMAND apkman include-paths-cmake
  COMMAND_ERROR_IS_FATAL ANY
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE APKMAN_INCLUDE_PATHS)

execute_process(
  COMMAND apkman lib-paths-cmake
  COMMAND_ERROR_IS_FATAL ANY
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE APKMAN_LIB_PATHS)


message("apkman include paths: ${APKMAN_INCLUDE_PATHS}")

add_enclave(
  TARGET
  demo6_enc
  UUID
  71b0822f-42a3-4543-a97c-ca491f76b82c
  SOURCES
  enc.cpp
  stubs.c
  ${CMAKE_CURRENT_BINARY_DIR}/demo_t.c)

enclave_include_directories(demo6_enc
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${APKMAN_INCLUDE_PATHS}

  # For using custom built lua
  #TODO: fetch path via apkman
  #~/.apkman/alpine-fs/apkbuild/pkgs/usr/include/lua5.3
  )

enclave_compile_options(demo6_enc
   PRIVATE
   -Wno-implicit-int-conversion
   -Wno-bitwise-op-parentheses
   -Wno-shift-op-parentheses
   -Wno-shorten-64-to-32
   -Wno-implicit-float-conversion
   -Wno-sign-conversion
   -Wno-unused-parameter
   -Wno-unused-variable
)

enclave_link_libraries(
  demo6_enc
  ${APKMAN_LIB_PATHS}
  libevent_core.a
  oelibcxx
  oelibc
  oehostepoll
  oehostfs
  oehostsock
  -fuse-ld=lld
  -Wl,--warn-unresolved-symbols)
