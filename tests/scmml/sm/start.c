// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

/*
extern void (*__preinit_array_start []) (void) __attribute__((weak));
extern void (*__preinit_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void); // __attribute__((weak));
extern void (*__init_array_end []) (void); // __attribute__((weak));
extern void (*__fini_array_start []) (void) __attribute__((weak));
extern void (*__fini_array_end []) (void) __attribute__((weak));
*/

void _start()
{
    // Should this call the init functions?
}
