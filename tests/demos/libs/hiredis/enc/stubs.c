// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <string.h>

void * __memcpy_chk(void * dest, const void * src, size_t len, size_t destlen)
{
    if (len > destlen)
	oe_abort();
    return memcpy(dest, src, len);
}

void * __memset_chk(void * dest, int c, size_t len, size_t destlen)
{
    if (len > destlen)
	oe_abort();
    return memset(dest, c, len);
}

#define UNIMPLEMENTED_LIBC_FUNCTION(name) \
    void name();                          \
    void name()                           \
    {                                     \
        oe_abort();                       \
    }

UNIMPLEMENTED_LIBC_FUNCTION(dlopen)
UNIMPLEMENTED_LIBC_FUNCTION(fchown)    
#if 0
UNIMPLEMENTED_LIBC_FUNCTION(dlclose)
UNIMPLEMENTED_LIBC_FUNCTION(dlsym)
UNIMPLEMENTED_LIBC_FUNCTION(dlerror)
UNIMPLEMENTED_LIBC_FUNCTION(utimes)
UNIMPLEMENTED_LIBC_FUNCTION(localtime_r)
UNIMPLEMENTED_LIBC_FUNCTION(fchmod)
UNIMPLEMENTED_LIBC_FUNCTION(posix_fallocate)
UNIMPLEMENTED_LIBC_FUNCTION(mremap)
UNIMPLEMENTED_LIBC_FUNCTION(readlink)
UNIMPLEMENTED_LIBC_FUNCTION(lstat)

#endif
