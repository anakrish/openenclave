// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include "terminate_t.h"

oe_result_t enc_check_ocall_terminate()
{
    oe_result_t result = OE_OK;
    host_terminate(&result);
    return result;
}

void enc_wait_in_host()
{
    host_wait();
}

OE_SET_ENCLAVE_SGX(
    1,        /* ProductID */
    1,        /* SecurityVersion */
    true,     /* Debug */
    1024,     /* NumHeapPages */
    64,       /* NumStackPages */
    NUM_TCS); /* NumTCS */
