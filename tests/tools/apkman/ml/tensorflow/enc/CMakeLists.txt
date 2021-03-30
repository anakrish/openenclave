# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

set(EDL_FILE ../test.edl)

add_custom_command(
  OUTPUT test_t.h test_t.c
  DEPENDS ${EDL_FILE} edger8r
  COMMAND
    edger8r --trusted ${EDL_FILE} --search-path ${PROJECT_SOURCE_DIR}/include
    ${DEFINE_OE_SGX} --search-path ${CMAKE_CURRENT_SOURCE_DIR} --search-path
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../utils)

add_custom_target(
  tensorflow
  COMMAND apkman add linux-headers
  DEPENDS apkman-init)

add_custom_command(
  COMMAND apkman exec /bin/bash ${CMAKE_CURRENT_SOURCE_DIR}/build.bash
  DEPENDS apkman-init
  OUTPUT libtensorflow-lite.a libruy.a libcpuinfo.a libclog.a empty.c)

add_enclave(
  TARGET
  tensorflow_enc
  UUID
  71b0822f-42a3-4543-a97c-ca491f76b82c
  SOURCES
  enc.c
  stubs.c
  empty.c
  bitmap_helpers.cc
  label_image.cc
  tflite_misc.cc
  ${CMAKE_CURRENT_BINARY_DIR}/test_t.c)

add_dependencies(tensorflow_enc tensorflow)

enclave_include_directories(
  tensorflow_enc
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  # tensorflow include path.
  ${CMAKE_CURRENT_BINARY_DIR}/tensorflow
  ${CMAKE_CURRENT_BINARY_DIR}/build/flatbuffers/include
  ${CMAKE_CURRENT_BINARY_DIR}/build/abseil-cpp
  ${APKMAN_ROOT}/usr/include/c++/10.2.1
  ${APKMAN_ROOT}/usr/include/c++/10.2.1/x86_64-alpine-linux-musl
  ${APKMAN_ROOT}/usr/include)

enclave_compile_options(
  tensorflow_enc
  PRIVATE
  -DTFLITE_WITHOUT_XNNPACK
  -Wno-implicit-int-conversion
  -Wno-bitwise-op-parentheses
  -Wno-shift-op-parentheses
  -Wno-shorten-64-to-32
  -Wno-implicit-float-conversion
  -Wno-unused-parameter
  -Wno-sign-conversion
  -Wno-float-conversion
  -Wno-sign-compare)

enclave_link_libraries(
  tensorflow_enc
  -L${CMAKE_CURRENT_BINARY_DIR}
  libtensorflow-lite.a
  libruy.a
  libcpuinfo.a
  libclog.a
  -L${APKMAN_ROOT}/usr/lib
  libgcc.a
  libstdc++.a
  libunwind
  oelibc
  oehostfs
  oeencpthread
  -Wl,--warn-unresolved-symbols)
