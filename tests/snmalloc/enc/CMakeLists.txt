# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

set(EDL_FILE ../snmalloc.edl)

add_custom_command(
  OUTPUT snmalloc_t.h snmalloc_t.c
  DEPENDS ${EDL_FILE} edger8r
  COMMAND edger8r --trusted ${EDL_FILE} --search-path
          ${CMAKE_CURRENT_SOURCE_DIR})

add_library(snmalloc_test_lib $<TARGET_OBJECTS:oesnmalloc>)

add_enclave(
  TARGET
  snmalloc_enc
  UUID
  e024b61f-8599-499d-bd83-b4b9da9dd389
  SOURCES
  enc.c
  ${CMAKE_CURRENT_BINARY_DIR}/snmalloc_t.c)

enclave_include_directories(snmalloc_enc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
enclave_link_libraries(snmalloc_enc snmalloc_test_lib oelibc)
