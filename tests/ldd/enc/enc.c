// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <stdio.h>
#include <stdlib.h>

#include "ldd_t.h"

// SymCrypt callbacks
void* SymCryptCallbackAlloc(size_t n)
{
    return malloc(n);    
}

void SymCryptCallbackFree(void* ptr)
{
    return free(ptr);
}

#define SYMCRYPT_ERROR long
SYMCRYPT_ERROR SymCryptCallbackRandom(uint8_t* buffer, size_t buffer_size)
{
    if (oe_random(buffer, buffer_size) == 0)
        return 0; // SYMCRYPT_NO_ERROR
    // TODO: proper error code
    return -1;    
}
// qsort provided by C library

int SymCryptInit(void);


void test_SymCrypt(void)
{
    uint8_t data[4 * 1024];

    SymCryptInit();
    printf("Initialized symcrypt\n");
    
    oe_random(data, sizeof(data));

    printf("Symcrypt test passed\n");
}

int multiply_local_const_1a(int a);
__attribute__((weak)) int add_global_1a(int a, int b);
__attribute__((weak)) int test_nested_dependencies();

__attribute__((weak)) int multiply_local_const_1b(int a);
__attribute__((weak)) int add_global_1b(int a, int b);

__attribute__((weak)) int unlinked_function(int a, int b);

int enc_global = 0;
__attribute__((constructor)) static void my_constructor(void)
{
    enc_global = 1;
}


int test_enclave()
{
    int failed_tests = 0;
    
    if (enc_global != 1)
    {
        printf("enc_global was not initialized (%d)\n", enc_global);
        failed_tests++;
	//my_func();
    }

    /*
     * Test dep_1a functions
     */
    const int expected = 10010;
    int value = multiply_local_const_1a(10);
    printf(
        "multiply_local_const_1a(10) = %d, expected = %d\n", value, expected);
    if (value != expected)
        failed_tests++;

    if (add_global_1a)
    {
        const int expected = 11111;
        int value = add_global_1a(1000, 100);
        printf("add_global_1a(10) = %d, expected = %d\n", value, expected);
        if (value != expected)
            failed_tests++;
    }
    else
    {
        printf("add_global_1a not defined\n");
        failed_tests++;
    }

    if (test_nested_dependencies)
    {
        int value = test_nested_dependencies();
        printf("test_nested_dependencies() failures = %d\n", value);
        failed_tests += value;
    }
    else
    {
        printf("test_nested_dependencies not defined\n");
        failed_tests++;
    }

    /*
     * Test dep_1b functions
     */
    if (multiply_local_const_1b)
    {
        const int expected = 10020;
        int value = multiply_local_const_1b(10);
        printf(
            "multiply_local_const_1b(10) = %d, expected = %d\n",
            value,
            expected);
        if (value != expected)
            failed_tests++;
    }
    else
    {
        printf("multiply_local_const_1b not defined\n");
        failed_tests++;
    }

    if (add_global_1b)
    {
        const int expected = 11121;
        int value = add_global_1b(1000, 100);
        printf("add_global_1b(10) = %d, expected = %d\n", value, expected);
        if (value != expected)
            failed_tests++;
    }
    else
    {
        printf("add_global_1b not defined\n");
        failed_tests++;
    }

    /*
     * Negative test for unbound functions
     */
    if (unlinked_function)
    {
        printf("Found unexpected unlinked_function\n");
        failed_tests++;
    }
    else
        printf("unlinked_function is correctly not found\n");

    test_SymCrypt();
    return failed_tests;
}

OE_SET_ENCLAVE_SGX(
    1,    /* ProductID */
    1,    /* SecurityVersion */
    true, /* Debug */
    32 * 1024, /* NumHeapPages */
    1024, /* NumStackPages */
    2);   /* NumTCS */
