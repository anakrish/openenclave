// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.
#include <stdint.h>
#include <stdlib.h>

#define EXPORT __attribute__((visibility("default")))

__thread char ch = 1;
__thread int l = 800;

int foo(int a)
{
    return a * a;
}

int k = 500;

__attribute__((constructor)) static void my_constructor(void)
{
    k = 800;
}

__attribute__((destructor)) static void my_destructor(void)
{
    k = 400;
}

int add(int a, int b)
{
    int tmp = a + b + k;
    int tl = l;
    int tch = ch;
    return tmp - tl - tch;
}

__attribute__((weak)) void* enc_malloc(size_t n);
__attribute__((weak)) void enc_free(void* p);

// wrap enc_malooc and enc_free
// Only if they are tested, they become GLOB_DAT relocation,
// otherwise they just become JUMP_SLOT relocaton.

void* malloc(size_t n)
{
    if (enc_malloc)
        return enc_malloc(n);
    return 0;
}

void free(void* p)
{
    if (enc_free)
        enc_free(p);
}

void test_malloc(void)
{
    void* p = enc_malloc(1024);
    free(p);
}
