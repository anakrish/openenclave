# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

set(EDL_FILE ../demo.edl)

add_custom_command(
  OUTPUT demo_t.h demo_t.c
  DEPENDS ${EDL_FILE} edger8r
  COMMAND
    edger8r --trusted ${EDL_FILE} --search-path ${PROJECT_SOURCE_DIR}/include
    ${DEFINE_OE_SGX} --search-path ${CMAKE_CURRENT_SOURCE_DIR})


set(PROTO_FILE addressbook.proto)
add_custom_command(
  OUTPUT addressbook.pb.h addressbook.pb.cc
  COMMAND apkman add protoc protobuf-dev
  COMMAND
      apkman exec protoc --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/..
                         --cpp_out=. ${PROTO_FILE}
)

# STEP 1: Obtain apkman root path.
execute_process(
  COMMAND apkman root
  COMMAND_ERROR_IS_FATAL ANY
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE APKMAN_ROOT)

add_enclave(
  TARGET
  protobuf_enc
  UUID
  71b0822f-42a3-4543-a97c-ca491f76b82c
  SOURCES
  enc.cpp
  # STEP 5: Implement missing functions.
  # stubs.c
  ${CMAKE_CURRENT_BINARY_DIR}/demo_t.c
  addressbook.pb.cc)

enclave_include_directories(protobuf_enc
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  # STEP 2: Add include paths.
  # Protobuf headers available via protobuf-dev library.
  # C++
  ${APKMAN_ROOT}/usr/include/c++/10.2.1
  ${APKMAN_ROOT}/usr/include/c++/10.2.1/x86_64-alpine-linux-musl
  # C
  ${APKMAN_ROOT}/usr/include)

enclave_compile_options(protobuf_enc
   PRIVATE
   -Wno-shorten-64-to-32
   -Wno-bitwise-op-parentheses
   -Wno-shift-op-parentheses
   -Wno-sign-conversion
   -Wno-unused-parameter
   -Wno-implicit-int-conversion
)

target_link_libraries( 
  protobuf_enc
  # STEP 3: Add library path and libraries.
  # Available via protobuf-dev library.
  -L${APKMAN_ROOT}/apkbuild/pkgs/usr/lib
  -L${APKMAN_ROOT}/usr/lib
  # We can support full version of protobuf
  libprotobuf.a
  libstdc++.a
  oelibcxx # For libunwind which has been patched for enclaves
  oelibc
  oehostfs
  # Step 4: Warn unresolved symbols
  -Wl,--warn-unresolved-symbols)
 
