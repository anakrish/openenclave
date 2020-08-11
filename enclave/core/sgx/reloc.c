// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <openenclave/internal/elf.h>
#include <openenclave/internal/globals.h>
#include "init.h"

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

static void _apply_relocations(
    const uint8_t* baseaddr,
    const elf64_rela_t* relocs,
    size_t nrelocs)
{
    for (size_t i = 0; i < nrelocs; i++)
    {
        const elf64_rela_t* p = &relocs[i];

        /* If zero-padded bytes reached */
        if (p->r_offset == 0)
            break;

        /* Compute address of reference to be relocated */
        uint64_t* dest = (uint64_t*)(baseaddr + p->r_offset);

        uint64_t reloc_type = ELF64_R_TYPE(p->r_info);

        /* Relocate the reference */
        if (reloc_type == R_X86_64_RELATIVE)
        {
            *dest = (uint64_t)(baseaddr + p->r_addend);
        }
        else if (reloc_type == R_X86_64_GLOB_DAT)
        {
            int64_t addend = p->r_addend;
            if (addend)
            {
                *dest = (uint64_t)(baseaddr + p->r_addend);
            }
        }
    }
}

bool oe_apply_relocations(void)
{
    const elf64_rela_t* relocs = (const elf64_rela_t*)__oe_get_reloc_base();
    size_t nrelocs = __oe_get_reloc_size() / sizeof(elf64_rela_t);
    const uint8_t* baseaddr = (const uint8_t*)__oe_get_enclave_base();

    // Apply relocations for base image.
    _apply_relocations(baseaddr, relocs, nrelocs);

    // Apply relocations for secondary image.
    // Relocations can be applied in any order.
    for (size_t i = 0; i < OE_MAX_NUM_MODULES; ++i)
    {
        const oe_module_link_info_t* link_info = &oe_linked_modules[i];
        // Check if module is valid. Reloc rva will be non zero.
        if (link_info->reloc_rva)
        {
            const elf64_rela_t* relocs =
                (const elf64_rela_t*)(baseaddr + link_info->reloc_rva);
            size_t nrelocs = link_info->reloc_size / sizeof(elf64_rela_t);

            _apply_relocations(
                (const uint8_t*)(baseaddr + link_info->base_rva),
                relocs,
                nrelocs);
        }
    }

    return true;
}
