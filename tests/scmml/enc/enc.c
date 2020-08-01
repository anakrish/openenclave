// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <stdio.h>

#define OE_DYNLINK_SHARED_LIBRARY(name)                           \
    extern void* __start_oemoduls;                                \
    __attribute__((section("oemoduls"))) char cryptolib[] = name; \
    __attribute__((used)) void* oe_secondary_module_name(void)    \
    {                                                             \
        return __start_oemoduls;                                  \
    }

OE_DYNLINK_SHARED_LIBRARY("../sm/libsm.so")

__attribute__((weak)) int foo(int a);
__attribute__((weak)) int add(int a, int b);
__attribute__((weak)) int sub(int a, int b);

void enc_call_foo()
{
    if (foo)
    {
        printf("foo function defined in secondary module\n");
        int value = foo(8);
        printf("foo(8) = %d\n", value);
    }
    else
    {
        printf("foo not defined\n");
    }

    if (add)
    {
        printf("add function defined in secondary module\n");
        int value = add(8, 7);
        printf("add(8, 7) = %d (add adds 500 to result)\n", value);
    }
    else
    {
        printf("add not defined\n");
    }

    if (sub)
    {
        printf("sub function defined in secondary module\n");
        int value = sub(8, 7);
        printf("sub(8, 7) = %d\n", value);
    }
    else
    {
        printf("sub not defined\n");
    }
}

OE_SET_ENCLAVE_SGX(
    1,        /* ProductID */
    1,        /* SecurityVersion */
    true,     /* Debug */
    3 * 1024, /* NumHeapPages */
    64,       /* NumStackPages */
    2);       /* NumTCS */
