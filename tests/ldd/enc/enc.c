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

    //SymCryptInit();
    printf("Initialized symcrypt\n");
    
    oe_random(data, sizeof(data));

    printf("Symcrypt test passed\n");
}

void OPENSSL_init_crypto(void);

void my_foo(void);

static int callback(void *data, int argc, char **argv, char **azColName){
    int i;
    fprintf(stderr, "%s: ", (const char*)data);
   
    for(i = 0; i<argc; i++){
	printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
   
    printf("\n");
    return 0;
}

OE_EXPORT
void* dlopen(const char* name, ...)
{
    OE_UNUSED(name);
    return 0;
}

void* TestAlpineLibs()
{
    extern double sin(double);
    static void* libcfcns[] = { 
	sin,
//	 strcspn,
//	 strstr,
    }; 

    //OPENSSL_init_crypto();
    my_foo();
    {
	typedef void* sqllite3;
	extern int sqlite3_open(const char*, sqllite3** db);
	extern int sqlite3_close(sqllite3* db);
	extern int sqlite3_exec(sqllite3* db, const char* cmd, void* callback,
				void* data, char** err);
	sqllite3* db = NULL;
	int rc= sqlite3_open(":memory:", &db);
	char* zErrMsg = NULL;

	printf("sqllite3_open returned %d\n", rc);

	const char*  sql = "CREATE TABLE COMPANY("  \
	    "ID INT PRIMARY KEY     NOT NULL," \
	    "NAME           TEXT    NOT NULL," \
	    "AGE            INT     NOT NULL," \
	    "ADDRESS        CHAR(50)," \
	    "SALARY         REAL );";	
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	if (rc!=0)
	    printf("sqllite3_exec returned %s", zErrMsg);
	else
	    printf("sqllite3_exec executed successfully!\n");
	/* Create SQL statement */
	sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
	    "VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
	    "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
	    "VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "     \
	    "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
	    "VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
	    "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
	    "VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	if (rc!=0)
	    printf("sqllite3_exec returned %s", zErrMsg);
	else
	    printf("sqllite3_exec executed successfully!\n");
 
	/* Create SQL statement */
	sql = "SELECT * from COMPANY";

	/* Execute SQL statement */
	const char* data = "Callback function called";
	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
	if (rc!=0)
	    printf("sqllite3_exec returned %s", zErrMsg);
	else
	    printf("sqllite3_exec executed successfully!\n");
	
        if (db)
	{
	    sqlite3_close(db);
	}
    }
    return libcfcns;
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

    //test_SymCrypt();
    TestAlpineLibs();
    return failed_tests;
}

OE_SET_ENCLAVE_SGX(
    1,    /* ProductID */
    1,    /* SecurityVersion */
    true, /* Debug */
    32 * 1024, /* NumHeapPages */
    1024, /* NumStackPages */
    2);   /* NumTCS */
