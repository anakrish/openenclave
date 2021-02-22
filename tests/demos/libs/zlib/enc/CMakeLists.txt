# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

set(EDL_FILE ../demo.edl)
set(PROTO_FILE ./geometry.proto)

add_custom_command(
  OUTPUT demo_t.h demo_t.c
  DEPENDS ${EDL_FILE} edger8r
  COMMAND
    edger8r --trusted ${EDL_FILE} --search-path ${PROJECT_SOURCE_DIR}/include
    ${DEFINE_OE_SGX} --search-path ${CMAKE_CURRENT_SOURCE_DIR})

# STEP 1: Obtain apkman root path.
execute_process(
  COMMAND apkman root
  COMMAND_ERROR_IS_FATAL ANY
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE APKMAN_ROOT)

add_enclave(
  TARGET
  zlib_enc
  UUID
  71b0822f-42a3-4543-a97c-ca491f76b82c
  SOURCES
  enc.cpp
  # STEP 5: Implement missing functions.
  # stubs.c
  ${CMAKE_CURRENT_BINARY_DIR}/demo_t.c)

enclave_include_directories(zlib_enc
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  # STEP 2: Add include paths.
  # Headers obtained by installing zlib-dev package.
  ${APKMAN_ROOT}/usr/include
  )

enclave_compile_options(zlib_enc
   PRIVATE
   -Wno-implicit-int-conversion
   -Wno-bitwise-op-parentheses
   -Wno-shift-op-parentheses
   -Wno-shorten-64-to-32
)

enclave_link_libraries(
  zlib_enc
  ${APKMAN_LIB_PATHS}
  libz.a
  oelibcxx
  oelibc
  oehostfs
  -Wl,--warn-unresolved-symbols)
