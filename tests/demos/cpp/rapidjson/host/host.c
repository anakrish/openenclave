// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <limits.h>
#include <openenclave/host.h>
#include <openenclave/internal/error.h>
#include <openenclave/internal/tests.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../host/strings.h"
#include "demo_u.h"

int host_echo(
    char* in,
    char* out,
    char* str1,
    char* str2,
    char* str3,
    int out_length)
{
    OE_TEST(strcmp(str1, "oe_host_strdup1") == 0);
    OE_TEST(strcmp(str2, "oe_host_strdup2") == 0);
    OE_TEST(strcmp(str3, "oe_host_strdup3") == 0);

    strcpy_s(out, out_length, in);

    return 0;
}

int main(int argc, const char* argv[])
{
    oe_result_t result;
    oe_enclave_t* enclave = NULL;
    int return_val;
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s ENCLAVE_PATH\n", argv[0]);
        return 1;
    }

    const uint32_t flags = oe_get_create_flags();

    if ((result = oe_create_demo_enclave(
             argv[1], OE_ENCLAVE_TYPE_SGX, flags, NULL, 0, &enclave)) != OE_OK)
        oe_put_err("oe_create_enclave(): result=%u", result);

    result = enc_demo(enclave, &return_val, "rapidjson-demo");

    if (result != OE_OK)
        oe_put_err("oe_call_enclave() failed: result=%u", result);

    if (return_val != 0)
        oe_put_err("ECALL failed args.result=%d", return_val);

    result = oe_terminate_enclave(enclave);
    OE_TEST(result == OE_OK);

    return 0;
}
