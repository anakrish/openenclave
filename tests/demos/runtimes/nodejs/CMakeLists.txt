# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

add_subdirectory(host)

if (BUILD_ENCLAVES)
  add_subdirectory(enc)
endif ()

add_enclave_test(tests/demos/nodejs_1 nodejs_host nodejs_enc
 ${CMAKE_CURRENT_SOURCE_DIR}/sieve.lua)

add_enclave_test(tests/demos/nodejs_2 nodejs_host nodejs_enc
 ${CMAKE_CURRENT_SOURCE_DIR}/bisect.lua)
