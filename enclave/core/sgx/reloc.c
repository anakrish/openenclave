// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <openenclave/internal/elf.h>
#include <openenclave/internal/globals.h>
#include "../tracee.h"
#include "init.h"

static bool _allow_unresolved_symbols()
{
    // Undefined symbols are allowed only for development.
    // Since this code is executed during relocation, dependencies are
    // kept to minimum.
    static bool allowed = false;
    static bool first_time = true;
    if (first_time)
    {
        allowed = oe_is_enclave_debug_allowed();
        first_time = false;
    }
    return allowed;
}

/*
**==============================================================================
**
** oe_apply_relocations()
**
**     Apply symbol relocations from the relocation pages, whose content
**     was copied from the ELF file during loading. These relocations are
**     included in the enclave signature (MRENCLAVE).
**
**==============================================================================
*/

bool oe_apply_relocations(void)
{
    const elf64_rela_t* relocs = (const elf64_rela_t*)__oe_get_reloc_base();
    size_t nrelocs = __oe_get_reloc_size() / sizeof(elf64_rela_t);
    const uint8_t* start_address =
        (const uint8_t*)__oe_get_enclave_start_address();

    for (size_t i = 0; i < nrelocs; i++)
    {
        const elf64_rela_t* p = &relocs[i];

        /* If zero-padded bytes reached */
        if (p->r_offset == 0)
            break;

        /* Compute address of reference to be relocated */
        uint64_t* dest = (uint64_t*)(start_address + p->r_offset);

        uint64_t reloc_type = ELF64_R_TYPE(p->r_info);

        /* Relocate the reference */
        if (reloc_type == R_X86_64_RELATIVE)
        {
            int64_t addend = p->r_addend;
            /* Process only if the symbol is defined */
            if (addend)
                *dest = (uint64_t)(start_address + p->r_addend);
        }
        else if (reloc_type == R_X86_64_64)
        {
            if (_allow_unresolved_symbols())
                continue;
            else
                oe_abort();
        }
    }

    return true;
}
