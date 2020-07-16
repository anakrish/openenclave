// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/bits/properties.h>
#include <openenclave/enclave.h>
#include <string.h>

// This test performs a lot of memory allocations and the memset for each
// allocation/free will cause the test to take more than 30 minutes to complete.
// Turn off clearing of memory under debug malloc by providing an empty
// implementation.
void oe_debug_malloc_clear_memory(void* p, size_t size)
{
    OE_UNUSED(p);
    OE_UNUSED(size);
}

OE_SET_ENCLAVE_SGX(
    1234, /* ProductID */
    5678, /* SecurityVersion */
    true, /* Debug */
#ifdef NO_PAGING_SUPPORT
    /* Set smaller heap for systems without paging support. */
    2560, /* NumHeapPages */
#else
    131072, /* NumHeapPages */
#endif
    32, /* NumStackPages */
    4); /* NumTCS */
