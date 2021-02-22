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

add_enclave(
  TARGET
  demo8_enc
  UUID
  71b0822f-42a3-4543-a97c-ca491f76b82c
  SOURCES
  enc.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/demo_t.c)

enclave_include_directories(demo8_enc
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  /home/anand/.apkman/alpine-fs/usr/include/c++/10.2.1
  /home/anand/.apkman/alpine-fs/usr/include/c++/10.2.1/x86_64-alpine-linux-musl
  /home/anand/.apkman/alpine-fs/usr/include
  # for intrinsics
  /home/anand/.apkman/alpine-fs/usr/lib/clang/10.0.1/include)

enclave_compile_options(demo8_enc
  PRIVATE
  -Wno-unused-parameter
  -Wno-shorten-64-to-32
  -Wno-shift-op-parentheses
  -Wno-implicit-int-conversion
  -Wno-bitwise-op-parentheses
  -Wno-sign-conversion
  -Wno-implicit-int-float-conversion
  -Wno-float-conversion
)

target_link_libraries( 
  demo8_enc
  ${APKMAN_LIB_PATHS}
  libboost_serialization.a
  libstdc++.a
  # for __umodti3
  libgcc.a
  oelibcxx # For libunwind which has been patched for enclaves
  oelibc
  -Wl,--warn-unresolved-symbols)
 
