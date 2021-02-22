# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

set(EDL_FILE ../demo.edl)

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
  sqlite_enc
  UUID
  71b0822f-42a3-4543-a97c-ca491f76b82c
  SOURCES
  enc.c
  # STEP 5: Implement missing functions.
  stubs.c
  ${CMAKE_CURRENT_BINARY_DIR}/demo_t.c)

enclave_include_directories(sqlite_enc
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  # STEP 2: Add include paths.
  # Headers obtained by installing sqlite3-dev package.
  ${APKMAN_ROOT}/usr/include)

enclave_link_libraries(
  sqlite_enc
  # STEP 3: Add library path and libraries.
  # Library obtained by installing sqlite3-static package.
  # -L${APKMAN_ROOT}/apkbuild/pkgs/usr/lib
  -L${APKMAN_ROOT}/usr/lib
  libsqlite3.a
  oelibc
  # STEP 4: Allow missing symbols for development.
  -Wl,--warn-unresolved-symbols)
