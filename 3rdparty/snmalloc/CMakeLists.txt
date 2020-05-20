# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

check_submodule_not_empty("${PROJECT_SOURCE_DIR}/3rdparty/snmalloc/snmalloc")
add_enclave_library(oesnmalloc OBJECT allocator.cpp)
add_dependencies(oesnmalloc libcxx_includes oelibc_includes)

if (OE_TRUSTZONE)
  enclave_link_libraries(oesnmalloc PUBLIC oelibutee_includes)
  set(TEE_C_FLAGS ${OE_TZ_TA_C_FLAGS})
else ()
  set(TEE_C_FLAGS -mcx16)
endif ()

enclave_compile_options(
  oesnmalloc
  PRIVATE
  -I${CMAKE_CURRENT_SOURCE_DIR}/snmalloc/src
  -I${PROJECT_SOURCE_DIR}/include
  -I${LIBCXX_INCLUDES}
  -I${PROJECT_BINARY_DIR}/output/include/openenclave/libc
  -Wno-conversion
  -ftls-model=local-exec
  -nostdinc++
  -nostdinc
  -fPIE
  -ffreestanding
  ${TEE_C_FLAGS}
  -fno-exceptions
  -fvisibility=hidden)

# Specify the following option as source files properties so that
# it will appear last in the compiler command line and supercede any
# other std specification.
set_source_files_properties(allocator.cpp COMPILE_FLAGS -std=c++17)

maybe_build_using_clangw(oesnmalloc)
