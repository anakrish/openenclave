// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include "demo_t.h"

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include <caml/callback.h>
#include <caml/major_gc.h>

extern int fib(int n);
extern char * format_result(int n);
extern void traverse();
extern void gc_stuff();

int enc_demo(char* in)
{
    if (strcmp(in, "sqlite-demo") != 0)
    {
        return -1;
    }

    {
	char* argv[] = {"demo1_host", NULL};
	// Use pooled startup to free statically allocated closures.
	caml_startup_pooled(argv);
	int x = fib(5);
	printf("ocaml fib returned %d\n", x);
	char* result = format_result(x);
	printf("formatted result: %s", result);
	free(result);
	traverse();
	// Print GC stats.
	gc_stuff();
	caml_shutdown();
    }

    printf("=== passed all tests (%s)\n", in);

    return 0;
}

char* secure_getenv()
{
    return NULL;
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz)
{
    OE_UNUSED(pathname);
    OE_UNUSED(buf);
    OE_UNUSED(bufsiz);
    sprintf(buf, "demo1_host");
    return (ssize_t)strlen(buf);
}

int sigsetjmp(sigjmp_buf b, int  mask)
{
    // TODO: signal mask
    OE_UNUSED(mask);
    return setjmp(*(jmp_buf*)(void*)b);
}

void siglongjmp(sigjmp_buf b, int ret)
{
    longjmp(*(jmp_buf*)(void*)b, ret);
}

OE_SET_ENCLAVE_SGX(
    1,    /* ProductID */
    1,    /* SecurityVersion */
    true, /* Debug */
    16*1024, /* NumHeapPages */
    1024, /* NumStackPages */
    2);   /* NumTCS */
