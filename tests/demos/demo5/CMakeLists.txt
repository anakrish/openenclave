# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

add_subdirectory(host)

if (BUILD_ENCLAVES)
  add_subdirectory(enc)
endif ()

add_enclave_test(tests/demos/demo5_1 demo5_host demo5_enc
 ${CMAKE_CURRENT_SOURCE_DIR}/sieve.lua)

add_enclave_test(tests/demos/demo5_2 demo5_host demo5_enc
 ${CMAKE_CURRENT_SOURCE_DIR}/bisect.lua)
