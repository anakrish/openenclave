// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/internal/tests.h>
#include <stdlib.h>

#include "snmalloc_t.h"

// snmalloc requires at least 4K heap pages.
#define MIN_NUM_HEAP_PAGES (4 * 1024)

void enc_test_snmalloc_basic()
{
    // Make sure basic allocation/free works.
    void* p = malloc(1024);
    OE_TEST(p != NULL);
    free(p);

    // Make sure allocation fails as expected.
    p = malloc(1024 * 1024 * 1024);
    OE_TEST(p == NULL);
}

OE_SET_ENCLAVE_SGX(
    1,                  /* ProductID */
    1,                  /* SecurityVersion */
    true,               /* Debug */
    MIN_NUM_HEAP_PAGES, /* NumHeapPages */
    1024,               /* NumStackPages */
    2);                 /* NumTCS */
