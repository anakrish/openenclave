// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

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
