// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <openenclave/internal/tests.h>

// For mounting host filesystem
#include <openenclave/internal/syscall/device.h>
#include <openenclave/internal/syscall/sys/mount.h>
#include <openenclave/internal/syscall/unistd.h>
#include <openenclave/internal/tests.h>
#include <sys/mount.h>

#include "demo_t.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void register_pthread_hooks(void);


//extern const char* old_tz;
extern int main(int argc, char** argv);

/* compress or decompress from stdin to stdout */
int enc_main(int argc, const char** argv)
{
    int ret = -1;
    bool mounted = false;

    OE_TEST(oe_load_module_host_file_system() == OE_OK);
    OE_TEST(oe_mount("/", "/", OE_DEVICE_NAME_HOST_FILE_SYSTEM, 0, NULL) == 0);
mounted = true;


    OE_TEST(oe_load_module_host_socket_interface() == OE_OK);
    OE_TEST(oe_load_module_host_epoll() == OE_OK);
    OE_TEST(oe_load_module_host_resolver() == OE_OK);
    register_pthread_hooks();

    //old_tz = "UTC";
    ret = main(argc, (char**)argv);
    goto done;
done:
    if (mounted)
        oe_umount("/");

    return ret;
}

OE_SET_ENCLAVE_SGX(
    1,    /* ProductID */
    1,    /* SecurityVersion */
    true, /* Debug */
    16*1024, /* NumHeapPages */
    256, /* NumStackPages */
    16);   /* NumTCS */
