// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <openenclave/internal/syscall/declarations.h>
#include <openenclave/internal/syscall/sys/syscall.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include <unistd.h>

#define MAP_FAILED ((void*)-1)

OE_DEFINE_SYSCALL6(SYS_mmap)
{
    void* addr = (void*)arg1;
    size_t length = (size_t)arg2;
    int prot = (int)arg3;
    int flags = (int)arg4;
    int fd = (int)arg5;
    long offset = arg6;

    OE_UNUSED(prot);
    OE_UNUSED(flags);

    if (addr == NULL && fd != -1 && offset == 0 && length != 0)
    {
        void* buffer = calloc(1, length);
        if (buffer)
        {
            read(fd, buffer, length);
            return (long)buffer;
        }
    }
    /* Always fail */
    errno = ENOSYS;
    return (long)MAP_FAILED;
}

// Fix OE's locale implementation which returns NULL.
// Default locale is C.
#include <locale.h>
static char _locale[256] = "C";
char* setlocale(int category, const char* locale)
{
    OE_UNUSED(category);
    if (locale == NULL)
        return _locale;
    sprintf(_locale, "%s", locale);
    return _locale;
}

#if 0
int pthread_condattr_init(void* ca)
{
    OE_UNUSED(ca);
    return -1;
}

int pthread_condattr_setclock(void* ca, long clk)
{
    OE_UNUSED(ca);
    OE_UNUSED(clk);
    return -1;
}

int sem_init(void* lock, int a, int b)
{
    OE_UNUSED(lock);
    OE_UNUSED(a);
    OE_UNUSED(b);
    return 0;
}

int readlink(void* path, void* cbuf, size_t len)
{
    return -1;
    OE_UNUSED(len);
    return swprintf(cbuf, len, L"%s", path);
}

int getrandom(void* buf, size_t buflen, unsigned int flags)
{
    OE_UNUSED(flags);
    oe_random(buf, buflen);
    return -1; // tell python that getrandom doesn't work
}

int sem_wait(void* sem)
{
    OE_UNUSED(sem);
    return 0;
}

int sem_post(void* sem)
{
    OE_UNUSED(sem);
    return 0;
}
#endif
