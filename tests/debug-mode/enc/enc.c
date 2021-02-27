// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/edger8r/enclave.h>
#include <openenclave/enclave.h>
#include "debug_mode_t.h"

bool oe_is_enclave_debug_allowed();

int test(void)
{
    return oe_is_enclave_debug_allowed();
}
