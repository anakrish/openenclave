// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <cstdio>
#include "test_t.h"

struct S
{
    int x;
    S() : x(8)
    {
    }
    ~S()
    {
        printf("S::~() called\n");
    }
};

thread_local S tls_s;

int enc_main(int argc, char** argv)
{
    OE_UNUSED(argc);
    OE_UNUSED(argv);
    return tls_s.x;
}

/* Prevent link error */
const void* __dso_handle = NULL;

OE_SET_ENCLAVE_SGX(
    1,    /* ProductID */
    1,    /* SecurityVersion */
    true, /* Debug */
    1024, /* NumHeapPages */
    1024, /* NumStackPages */
    2);   /* NumTCS */
