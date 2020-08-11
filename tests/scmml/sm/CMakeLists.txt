# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

add_library(sm SHARED sm.c start.c)

target_compile_options(
  sm
  PUBLIC -fPIC -nostdinc -fstack-protector-strong
         # Preserve frame-pointer in Release mode to enable oe_backtrace.
         -fno-omit-frame-pointer)

target_link_libraries(
  sm
  PRIVATE -nostdlib
          -nodefaultlibs
          -nostartfiles
          # Removed -Bsymbolic to trigger generation of relocations.
          # Removed -pie to create a shared library and instead used pic.
          -Wl,--no-undefined,-Bstatic,--export-dynamic,-pic,--build-id
          -Wl,-z,noexecstack
          -Wl,-z,now
          -Wl,-gc-sections
          -Wl,-entry=_start)
