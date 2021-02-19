// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>

// For mounting host filesystem
#include <openenclave/internal/syscall/device.h>
#include <openenclave/internal/syscall/sys/mount.h>
#include <openenclave/internal/syscall/unistd.h>
#include <openenclave/internal/tests.h>
#include <sys/mount.h>

#include "demo_t.h"

#include <Python.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* compress or decompress from stdin to stdout */
int enc_main(int argc, const char** argv)
{
    OE_UNUSED(argc);
    OE_UNUSED(argv);
    int ret = -1;
    bool mounted = false;

    OE_TEST(oe_load_module_host_file_system() == OE_OK);
    OE_TEST(oe_mount("/", "/", OE_DEVICE_NAME_HOST_FILE_SYSTEM, 0, NULL) == 0);
    mounted = true;

    Py_SetProgramName(L"enc-python");
    Py_Initialize();

    ret = 0;
    goto done;
done:
    if (mounted)
        oe_umount("/");

    return ret;
}

OE_SET_ENCLAVE_SGX(
    1,     /* ProductID */
    1,     /* SecurityVersion */
    true,  /* Debug */
    65536, /* NumHeapPages */
    1024,  /* NumStackPages */
    2);    /* NumTCS */
