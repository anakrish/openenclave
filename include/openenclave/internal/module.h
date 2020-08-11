// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#ifndef _OE_MODULE_H
#define _OE_MODULE_H

#include <openenclave/bits/defs.h>
#include <openenclave/bits/result.h>
#include <openenclave/bits/types.h>

OE_EXTERNC_BEGIN

typedef struct _oe_module_link_info
{
    uint64_t base_rva;

    // TODO: Should all relocations be combined?
    uint64_t reloc_rva;
    uint64_t reloc_size;

    // TODO: Use these values.
    uint64_t tdata_rva;
    uint64_t tdata_size;
    uint64_t tdata_align;
    uint64_t tbss_size;
    uint64_t tbss_align;

    // Init and fini sections.
    // TODO: Do we need preinit section?
    uint64_t init_array_rva;
    uint64_t init_array_size;
    uint64_t fini_array_rva;
    uint64_t fini_array_size;

    // TODO: Do we need entry point address?
    uint64_t entry_rva;
} oe_module_link_info_t;

OE_EXTERNC_END

#endif // _OE_MODULE_H
