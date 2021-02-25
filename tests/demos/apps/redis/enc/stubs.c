// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <errno.h>

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

long umask(long n)
{
    return n;
}

int pipe(int fd[2])
{
    OE_UNUSED(fd);
    return 0;
}

int getrlimit(int resource, struct rlimit *rlim)
{
    switch (resource)
    {
    case RLIMIT_NOFILE:
	rlim->rlim_cur = RLIM_INFINITY;
	rlim->rlim_max = RLIM_INFINITY;
	return 0;
    }
    return 0;
}


int pthread_attr_init(void* attr, long a)
{
    OE_UNUSED(attr);
    OE_UNUSED(a);
    return 0;
}

int pthread_attr_getstacksize(void* attr)
{
    OE_UNUSED(attr);
    return 16384;
}

int pthread_attr_setstacksize(void* attr, long s)
{
    OE_UNUSED(attr);
    OE_UNUSED(s);
    return 0;
}

void pthread_setname_np(void* t, const char* n)
{
    OE_UNUSED(t);
    OE_UNUSED(n);
}

int pthread_sigmask(int how, const sigset_t *restrict set, sigset_t *restrict old)
{
    OE_UNUSED(how);
    OE_UNUSED(set);
    OE_UNUSED(old);
    return 0;
}

// for reading time zone
char* __map_file(const char* filename, size_t* size)
{
    size_t sz = 0;
    FILE* fp = fopen(filename, "rb");
    fseek(fp, 0L, SEEK_END);
    sz = (size_t)ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    char* buf = (char*)malloc(sz);
    fread(buf, 1, sz, fp);
    fclose(fp);
    *size = sz;
    return buf;
}
