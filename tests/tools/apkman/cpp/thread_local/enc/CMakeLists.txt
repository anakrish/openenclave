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

add_enclave(
  TARGET
  apk_tls_enc
  UUID
  71b0822f-42a3-4543-a97c-ca491f76b82c
  SOURCES
  enc.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/test_t.c)

enclave_include_directories(apk_tls_enc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(
  apk_tls_enc
  -nostdlib
  # Add library path and libraries.
  -L${APKMAN_ROOT}/usr/lib
  # Specify oecore before libstdc++ so that OE's implementation
  # of __cxa_thread_atexit will be picked up by the linker.
  # oecore is also added as the last archive in the linker line
  # due to specification of oelibc.
  oecore
  libstdc++.a
  libunwind # For libunwind which has been patched for enclaves
  oelibc
  # Use apkman's ld to avoid bug with Ubuntu 18.04's binutils.
  -fuse-ld=${CMAKE_CURRENT_SOURCE_DIR}/../../../utils/ld
  # Emit warnings for unresolved symbols.
  -Wl,--warn-unresolved-symbols)
