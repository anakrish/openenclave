// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <stdlib.h>

#if 0
#define UNIMPLEMENTED_LIBC_FUNCTION(name) \
    void name();                          \
    void name()                           \
    {                                     \
        oe_abort();                       \
    }

UNIMPLEMENTED_LIBC_FUNCTION(dlopen)
UNIMPLEMENTED_LIBC_FUNCTION(dlclose)
UNIMPLEMENTED_LIBC_FUNCTION(dlsym)
UNIMPLEMENTED_LIBC_FUNCTION(dlerror)
UNIMPLEMENTED_LIBC_FUNCTION(utimes)
UNIMPLEMENTED_LIBC_FUNCTION(localtime_r)
UNIMPLEMENTED_LIBC_FUNCTION(fchmod)
UNIMPLEMENTED_LIBC_FUNCTION(posix_fallocate)
UNIMPLEMENTED_LIBC_FUNCTION(fchown)
UNIMPLEMENTED_LIBC_FUNCTION(mremap)
UNIMPLEMENTED_LIBC_FUNCTION(readlink)
UNIMPLEMENTED_LIBC_FUNCTION(lstat)

#endif
