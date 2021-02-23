// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <openenclave/internal/tests.h>
#include <openenclave/debugmalloc.h>
#include "demo_t.h"

#include <cstdio>
#include <cstring>

#include <openenclave/internal/syscall/device.h>
#include <openenclave/internal/syscall/sys/mount.h>

extern "C" int main(int argc, const char** argv);

int enc_main(int argc, const char** argv)
{
    // ooeeger8r does not release any memory.
    oe_debug_malloc_tracking_start();


    int ret = -1;
    bool mounted = false;

    OE_TEST(oe_load_module_host_file_system() == OE_OK);
    OE_TEST(oe_mount("/", "/", OE_DEVICE_NAME_HOST_FILE_SYSTEM, 0, NULL) == 0);
    mounted = true;

    ret = main(argc, argv);
    oe_debug_malloc_tracking_stop();
    
    if (mounted)
	oe_umount("/");
    return ret;
}

extern "C" int system(const char* cmd)
{
    printf("sytem not implemented\n");
    return 0;
}

OE_SET_ENCLAVE_SGX(
    1,    /* ProductID */
    1,    /* SecurityVersion */
    true, /* Debug */
    16*1024, /* NumHeapPages */
    1024, /* NumStackPages */
    2);   /* NumTCS */
