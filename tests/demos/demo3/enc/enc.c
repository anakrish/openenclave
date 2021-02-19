// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include "demo_t.h"

#include <stdio.h>
#include <string.h>

#include <complex.h>
double complex foo(double complex a, double complex b)
{
    return a * b;
}

complex double g = 1;
complex double h = 1;
int enc_demo(char* in)
{
    foo(g, h);

    printf("=== passed all tests (%s)\n", in);
    return 0;
}

OE_SET_ENCLAVE_SGX(
    1,    /* ProductID */
    1,    /* SecurityVersion */
    true, /* Debug */
    1024, /* NumHeapPages */
    1024, /* NumStackPages */
    2);   /* NumTCS */
