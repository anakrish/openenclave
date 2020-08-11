// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <assert.h>
#include <errno.h>
#include <openenclave/bits/defs.h>
#include <openenclave/bits/sgx/sgxtypes.h>
#include <openenclave/host.h>
#include <openenclave/internal/calls.h>
#include <openenclave/internal/load.h>
#include <openenclave/internal/mem.h>
#include <openenclave/internal/module.h>
#include <openenclave/internal/properties.h>
#include <openenclave/internal/raise.h>
#include <openenclave/internal/safecrt.h>
#include <openenclave/internal/safemath.h>
#include <openenclave/internal/sgx/td.h>
#include <openenclave/internal/sgxcreate.h>
#include <openenclave/internal/trace.h>
#include <openenclave/internal/utils.h>
#include <stdlib.h>
#include <string.h>
#include "../memalign.h"
#include "../strings.h"
#include "enclave.h"
#include "sgxload.h"

static oe_result_t _free_elf_image(oe_enclave_image_t* image)
{
    if (image->u.elf.elf.data)
    {
        free(image->u.elf.elf.data);
    }

    if (image->image_base)
    {
        oe_memalign_free(image->image_base);
    }

    if (image->u.elf.segments)
    {
        oe_memalign_free(image->u.elf.segments);
    }

    memset(image, 0, sizeof(*image));

    return OE_OK;
}

static int _compare_segments(const void* s1, const void* s2)
{
    const oe_elf_segment_t* seg1 = (const oe_elf_segment_t*)s1;
    const oe_elf_segment_t* seg2 = (const oe_elf_segment_t*)s2;

    return (int)(seg1->vaddr - seg2->vaddr);
}

static oe_result_t _load_elf_image(
    const char* path,
    oe_enclave_image_t* image,
    bool primary)
{
    oe_result_t result = OE_UNEXPECTED;
    size_t i;
    const elf64_ehdr_t* eh;
    size_t num_segments;
    bool has_build_id = false;

    assert(image && path);

    memset(image, 0, sizeof(*image));

    if (elf64_load(path, &image->u.elf.elf) != 0)
    {
        OE_RAISE(OE_FAILURE);
    }

    /* Save pointer to header for convenience */
    eh = (elf64_ehdr_t*)image->u.elf.elf.data;

/* Fail if not a dynamic object */
#if 0
    if (eh->e_type != ET_DYN)
        OE_RAISE(OE_FAILURE);
#endif

    /* Fail if not Intel X86 64-bit */
    if (eh->e_machine != EM_X86_64)
        OE_RAISE_MSG(OE_FAILURE, "elf image is not Intel X86 64-bit", NULL);

    /* Fail if image is relocatable */
    if (eh->e_type == ET_REL)
        OE_RAISE_MSG(OE_FAILURE, "elf image is relocatable", NULL);

    /* Save entry point address */
    image->entry_rva = eh->e_entry;

    /* Find the addresses of the ".text", ".ecall", and ".oeinfo" sections */
    {
        for (i = 0; i < eh->e_shnum; i++)
        {
            const elf64_shdr_t* sh =
                elf64_get_section_header(&image->u.elf.elf, i);

            /* Invalid section header. The elf file is corrupted. */
            if (sh == NULL)
                OE_RAISE(OE_FAILURE);

            const char* name =
                elf64_get_string_from_shstrtab(&image->u.elf.elf, sh->sh_name);

            if (name)
            {
                if (strcmp(name, ".text") == 0)
                {
                    image->text_rva = sh->sh_addr;
                }
                else if (strcmp(name, ".oeinfo") == 0)
                {
#if 0
                    /* .oeinfo must contain exactly the property */
                    if (sh->sh_size != sizeof(oe_sgx_enclave_properties_t))
                    {
                        OE_RAISE(OE_FAILURE);
                    }
#endif
                    image->oeinfo_rva = sh->sh_addr;
                    image->oeinfo_file_pos = sh->sh_offset;
                    OE_TRACE_VERBOSE(
                        "Found properties block offset %lx size %lx",
                        sh->sh_offset,
                        sh->sh_size);
                }

                else if (strcmp(name, ".note.gnu.build-id") == 0)
                {
                    has_build_id = true;
                }
                else if (strcmp(name, ".tdata") == 0)
                {
                    // These items must match program header values.
                    image->tdata_rva = sh->sh_addr;
                    image->tdata_size = sh->sh_size;
                    image->tdata_align = sh->sh_addralign;

                    OE_TRACE_VERBOSE(
                        "loadelf: tdata { rva=%lx, size=%lx, align=%ld }\n",
                        sh->sh_addr,
                        sh->sh_size,
                        sh->sh_addralign);
                }
                else if (strcmp(name, ".tbss") == 0)
                {
                    image->tbss_size = sh->sh_size;
                    image->tbss_align = sh->sh_addralign;
                    OE_TRACE_VERBOSE(
                        "loadelf: tbss { size=%ld, align=%ld }\n",
                        sh->sh_size,
                        sh->sh_addralign);
                }
            }
        }

        /* Fail if required sections not found */
        if ((0 == image->text_rva) || (primary && 0 == image->oeinfo_rva))
        {
            OE_RAISE(OE_FAILURE);
        }

        /* It is now the default for linux shared libraries and executables to
         * have the build-id note. GCC by default passes the --build-id option
         * to linker, whereas clang does not. Build-id is also used as a key by
         * debug symbol-servers. If no build-id is found emit a trace message.
         * */
        if (!has_build_id)
        {
            OE_TRACE_ERROR("loadelf: enclave image does not have build-id.\n");
        }
    }

    /* Find out the image size and number of segments to be loaded */
    {
        uint64_t lo = 0xFFFFFFFFFFFFFFFF; /* lowest address of all segments */
        uint64_t hi = 0;                  /* highest address of all segments */

        for (i = 0; i < eh->e_phnum; i++)
        {
            const elf64_phdr_t* ph =
                elf64_get_program_header(&image->u.elf.elf, i);

            /* Check for corrupted program header. */
            if (ph == NULL)
                OE_RAISE(OE_FAILURE);

            /* Check for proper sizes for the program segment. */
            if (ph->p_filesz > ph->p_memsz)
                OE_RAISE(OE_FAILURE);

            switch (ph->p_type)
            {
                case PT_LOAD:

/* kind of surprised that segments may not be page aligned */
#if 0
                /* p_vaddr must be page aligned */
                if (ph->p_vaddr & (OE_PAGE_SIZE - 1))
                {
                    OE_RAISE(OE_FAILURE);
                }
#endif

                    if (lo > ph->p_vaddr)
                        lo = ph->p_vaddr;

                    if (hi < ph->p_vaddr + ph->p_memsz)
                        hi = ph->p_vaddr + ph->p_memsz;

                    image->u.elf.num_segments++;
                    break;

                default:
                    break;
            }
        }

        /* Fail if LO not found */
        if (lo != 0)
            OE_RAISE(OE_FAILURE);

        /* Fail if HI not found */
        if (hi == 0)
            OE_RAISE(OE_FAILURE);

        /* Fail if no segment found */
        if (image->u.elf.num_segments == 0)
            OE_RAISE(OE_FAILURE);

        /* Calculate the full size of the image (rounded up to the page size) */
        image->image_size = oe_round_up_to_page_size(hi - lo);
    }

    /* allocate segments */
    image->u.elf.segments = (oe_elf_segment_t*)oe_memalign(
        OE_PAGE_SIZE, image->u.elf.num_segments * sizeof(oe_elf_segment_t));
    memset(
        image->u.elf.segments,
        0,
        image->u.elf.num_segments * sizeof(oe_elf_segment_t));
    if (!image->u.elf.segments)
    {
        OE_RAISE(OE_OUT_OF_MEMORY);
    }

    /* Allocate image on a page boundary */
    image->image_base = (char*)oe_memalign(OE_PAGE_SIZE, image->image_size);
    if (!image->image_base)
    {
        OE_RAISE(OE_OUT_OF_MEMORY);
    }

    /* Clear the image memory */
    memset(image->image_base, 0, image->image_size);

    /* Add all loadable program segments to SEGMENTS array */
    for (i = 0, num_segments = 0; i < eh->e_phnum; i++)
    {
        const elf64_phdr_t* ph = elf64_get_program_header(&image->u.elf.elf, i);
        oe_elf_segment_t* seg = &image->u.elf.segments[num_segments];
        void* segdata;

        assert(ph);
        assert(ph->p_filesz <= ph->p_memsz);
        if (ph->p_type == PT_TLS)
        {
            // The ELF Handling for ELF Storage spec
            // (https://uclibc.org/docs/tls.pdf) says in page 4:
            //     "The section header is not usable; instead a new program
            //      header entry is created."
            // These assertions exist to understand those scenarios better.
            // Currently, in all cases except one, the section and program
            // header values are observed to be same.
            if (image->tdata_rva != ph->p_vaddr)
            {
                if (image->tdata_rva == 0)
                {
                    // The ELF has no thread local variables that are
                    // explicitly initialized. There for there is no .tdata
                    // section; only a .tbss section.
                    // In this case, the linker seems to put the address of the
                    // .tbss section in p_vaddr field; however it leaves the
                    // size zero. This seems to be strange linker behavior;
                    // we don't assert on it.
                    OE_TRACE_INFO("Ignoring .tdata_rva, p_vaddr mismatch for "
                                  "empty .tdata section");
                }
                else
                {
                    OE_TRACE_ERROR(
                        "loadelf: .tdata rva mismatch. Section value = %lx, "
                        "Program header value = 0x%lx\n",
                        image->tdata_rva,
                        ph->p_vaddr);
                    OE_RAISE(OE_FAILURE);
                }
            }
            if (image->tdata_size != ph->p_filesz)
            {
                // Always assert on size mismatch.
                OE_TRACE_ERROR(
                    "loadelf: .tdata_size mismatch. Section value = %lx, "
                    "Program "
                    "header value = 0x%lx\n",
                    image->tdata_size,
                    ph->p_filesz);
                OE_RAISE(OE_FAILURE);
            }
            continue;
        }

        /* Skip non-loadable program segments */
        if (ph->p_type != PT_LOAD)
            continue;

        /* Set oe_elf_segment_t.memsz */
        seg->memsz = ph->p_memsz;

        /* Set oe_elf_segment_t.filesz, IS THIS FIELD NEEDED??? */
        seg->filesz = ph->p_filesz;

        /* Set oe_elf_segment_t.offset. IS THIS FIELD NEEDED??? */
        seg->offset = ph->p_offset;

        /* Set oe_elf_segment_t.vaddr */
        seg->vaddr = ph->p_vaddr;

        /* Set oe_elf_segment_t.flags */
        {
            if (ph->p_flags & PF_R)
                seg->flags |= OE_SEGMENT_FLAG_READ;

            if (ph->p_flags & PF_W)
                seg->flags |= OE_SEGMENT_FLAG_WRITE;

            if (ph->p_flags & PF_X)
                seg->flags |= OE_SEGMENT_FLAG_EXEC;
        }

        /* Set oe_elf_segment_t.filedata  IS THE FIELD NEEDED??? */
        seg->filedata = (unsigned char*)image->u.elf.elf.data + seg->offset;

        /* Should we fail if elf64_get_segment failed??? */
        segdata = elf64_get_segment(&image->u.elf.elf, i);
        if (segdata)
        {
            /* copy the segment to image */
            memcpy(image->image_base + seg->vaddr, segdata, seg->filesz);
        }

        num_segments++;
    }

    assert(num_segments == image->u.elf.num_segments);

    /* sort the segment by vaddr. NOT SURE IF THIS IS NEED - IS ELF SEGMENTED
     * SORTED LIKE PE SECTIONS? */
    qsort(
        image->u.elf.segments,
        num_segments,
        sizeof(oe_elf_segment_t),
        _compare_segments);

    /* check that segments are valid */
    for (i = 0; i < image->u.elf.num_segments - 1; i++)
    {
        const oe_elf_segment_t* seg = &image->u.elf.segments[i];
        const oe_elf_segment_t* seg_next = &image->u.elf.segments[i + 1];
        if ((seg->vaddr + seg->memsz) >
            oe_round_down_to_page_size(seg_next->vaddr))
        {
            OE_RAISE(OE_OUT_OF_BOUNDS);
        }
    }

    image->u.elf.elf.magic = ELF_MAGIC;
    result = OE_OK;

done:

    if (result != OE_OK)
    {
        _free_elf_image(image);
    }
    return result;
}

OE_INLINE void _dump_relocations(const void* data, size_t size)
{
    const elf64_rela_t* p = (const elf64_rela_t*)data;
    size_t n = size / sizeof(elf64_rela_t);

    printf("=== Relocations:\n");

    for (size_t i = 0; i < n; i++, p++)
    {
        if (p->r_offset == 0)
            break;

        printf(
            "offset=%llu addend=%lld\n",
            OE_LLU(p->r_offset),
            OE_LLD(p->r_addend));
    }
}

static oe_result_t _calculate_size(
    const oe_enclave_image_t* image,
    size_t* image_size)
{
    *image_size = image->image_size + image->reloc_size;
    if (image->secondary_image)
        *image_size += image->secondary_image->image_size +
                       image->secondary_image->reloc_size;

    return OE_OK;
}

static oe_result_t _unload(oe_enclave_image_t* image)
{
    if (image->secondary_image)
        _unload(image->secondary_image);

    if (image->u.elf.reloc_data)
    {
        oe_memalign_free(image->u.elf.reloc_data);
    }

    return _free_elf_image(image);
}

// ------------------------------------------------------------------

/*
**==============================================================================
**
** Image layout:
**
**     NHEAP = number of heap pages
**     NSTACK = number of stack pages
**     NTCS = number of TCS objects
**     GUARD = an unmapped guard page
**
**     [PAGES]:
**         [PROGRAM-PAGES]
**         [HEAP-PAGE]*NHEAP
**         ( [GUARD] [STACK-PAGE]*NSTACK [TCS-PAGES]*1 ) * NTCS
**
**     [PROGRAM-PAGES]:
**         [CODE-PAGES]: flags=reg|x|r content=(ELF segment)
**         [DATA-PAGES]: flags=reg|w|r content=(ELF segment)
**
**     [RELOCATION-PAGES]:
**
**     [HEAP-PAGES]: flags=reg|w|r content=0x00000000
**
**     [THREAD-PAGES]:
**         [GUARD-PAGE]
**         [STACK-PAGES]: flags=reg|w|r content=0xCCCCCCCC
**         [GUARD-PAGE]
**         [TCS-PAGE]
**         [SSA1-PAGE1]: flags=reg|w|r content=0x00000000 SSA-slot 0
**         [SSA2-PAGE2]: flags=reg|w|r content=0x00000000 SSA-slot 1
**         [GUARD-PAGE]
**         [SEG1-PAGE]: flags=reg|w|r content=0x00000000 FS or GS segment
**         [SEG2-PAGE]: flags=reg|w|r content=0x00000000 FS or GS segment
**
**==============================================================================
*/

static uint64_t _make_secinfo_flags(uint32_t flags)
{
    uint64_t r = 0;

    if (flags & OE_SEGMENT_FLAG_READ)
        r |= SGX_SECINFO_R;

    if (flags & OE_SEGMENT_FLAG_WRITE)
        r |= SGX_SECINFO_W;

    if (flags & OE_SEGMENT_FLAG_EXEC)
        r |= SGX_SECINFO_X;

    return r;
}

static oe_result_t _add_relocation_pages(
    oe_sgx_load_context_t* context,
    uint64_t enclave_addr,
    uint64_t module_base,
    const void* reloc_data,
    const size_t reloc_size,
    uint64_t* vaddr)
{
    oe_result_t result = OE_UNEXPECTED;

    if (!context || !vaddr)
        OE_RAISE(OE_INVALID_PARAMETER);

    if (reloc_data && reloc_size)
    {
        const oe_page_t* pages = (const oe_page_t*)reloc_data;
        size_t npages = reloc_size / sizeof(oe_page_t);

        for (size_t i = 0; i < npages; i++)
        {
            uint64_t addr = module_base + *vaddr;
            uint64_t src = (uint64_t)&pages[i];
            uint64_t flags = SGX_SECINFO_REG | SGX_SECINFO_R;
            bool extend = true;

            OE_CHECK(oe_sgx_load_enclave_data(
                context, enclave_addr, addr, src, flags, extend));
            (*vaddr) += sizeof(oe_page_t);
        }
    }

    result = OE_OK;

done:
    return result;
}

static oe_result_t _add_segment_pages(
    oe_sgx_load_context_t* context,
    uint64_t enclave_addr,
    uint64_t module_addr,
    const oe_elf_segment_t* segment,
    void* image)
{
    oe_result_t result = OE_UNEXPECTED;
    uint64_t flags;
    uint64_t page_rva;
    uint64_t segment_end;

    assert(context);
    assert(segment);
    assert(image);

    /* Take into account that segment base address may not be page aligned */
    page_rva = oe_round_down_to_page_size(segment->vaddr);
    segment_end = segment->vaddr + segment->memsz;
    flags = _make_secinfo_flags(segment->flags);

    if (flags == 0)
    {
        /* should we fail or just skip? follow the old logic for now*/
        result = OE_OK;
        goto done;
    }

    flags |= SGX_SECINFO_REG;

    for (; page_rva < segment_end; page_rva += OE_PAGE_SIZE)
    {
        OE_CHECK(oe_sgx_load_enclave_data(
            context,
            enclave_addr,
            module_addr + page_rva,
            (uint64_t)image + page_rva,
            flags,
            true));
    }

    result = OE_OK;

done:
    return result;
}

/* Add image to enclave */
static oe_result_t _add_pages_impl(
    oe_enclave_image_t* image,
    oe_sgx_load_context_t* context,
    oe_enclave_t* enclave,
    uint64_t* vaddr,
    uint64_t module_base)
{
    oe_result_t result = OE_UNEXPECTED;
    size_t i;

    assert(context);
    assert(enclave);
    assert(image);
    assert(vaddr && *vaddr == 0);
    assert((image->image_size & (OE_PAGE_SIZE - 1)) == 0);
    assert(enclave->size > image->image_size);

    /* Add the program segments first */

    for (i = 0; i < image->u.elf.num_segments; i++)
    {
        OE_CHECK(_add_segment_pages(
            context,
            enclave->addr,
            module_base,
            &image->u.elf.segments[i],
            image->image_base));
    }

    *vaddr = image->image_size;

    /* Add the relocation pages (contain relocation entries) */
    OE_CHECK(_add_relocation_pages(
        context,
        enclave->addr,
        module_base,
        image->u.elf.reloc_data,
        image->reloc_size,
        vaddr));

    module_base += image->image_size + image->reloc_size;

    if (image->secondary_image)
    {
        uint64_t secondary_vaddr = 0;
        OE_CHECK(_add_pages_impl(
            image->secondary_image,
            context,
            enclave,
            &secondary_vaddr,
            module_base));
        *vaddr += secondary_vaddr;
    }

    result = OE_OK;

done:
    return result;
}

static oe_result_t _add_pages(
    oe_enclave_image_t* image,
    oe_sgx_load_context_t* context,
    oe_enclave_t* enclave,
    uint64_t* vaddr)
{
    return _add_pages_impl(image, context, enclave, vaddr, enclave->addr);
}

static oe_result_t _get_dynamic_symbol_rva(
    oe_enclave_image_t* image,
    const char* name,
    uint64_t* rva)
{
    oe_result_t result = OE_UNEXPECTED;
    elf64_sym_t sym = {0};

    if (!image || !name || !rva)
        OE_RAISE(OE_INVALID_PARAMETER);

    if (elf64_find_dynamic_symbol_by_name(&image->u.elf.elf, name, &sym) != 0)
        goto done;

    *rva = sym.st_value;
    result = OE_OK;
done:
    return result;
}

static oe_result_t _set_uint64_t_dynamic_symbol_value(
    oe_enclave_image_t* image,
    const char* name,
    uint64_t value)
{
    oe_result_t result = OE_UNEXPECTED;
    elf64_sym_t sym = {0};
    uint64_t* symbol_address = NULL;

    if (elf64_find_dynamic_symbol_by_name(&image->u.elf.elf, name, &sym) != 0)
        goto done;

    symbol_address = (uint64_t*)(image->image_base + sym.st_value);
    *symbol_address = value;

    result = OE_OK;
done:
    return result;
}

static oe_result_t _link_symbols(
    oe_enclave_image_t* image,
    oe_enclave_image_t** deps,
    uint64_t num_deps)
{
    oe_result_t result = OE_UNEXPECTED;

    const elf64_sym_t* symtab = NULL;
    size_t symtab_size = 0;
    if (elf64_get_dynamic_symbol_table(
            &image->u.elf.elf, &symtab, &symtab_size) != 0)
        goto done;

    elf64_rela_t* relocs = (elf64_rela_t*)image->u.elf.reloc_data;
    uint64_t nrelocs = image->reloc_size / sizeof(relocs[0]);
    for (size_t i = 0; i < nrelocs; i++)
    {
        elf64_rela_t* p = &relocs[i];

        /* If zero-padded bytes reached */
        if (p->r_offset == 0)
            break;

        uint64_t sym_idx = ELF64_R_SYM(p->r_info);
        uint64_t reloc_type = ELF64_R_TYPE(p->r_info);

        /* Fix links to secondary module */
        if (reloc_type == R_X86_64_GLOB_DAT)
        {
            const elf64_sym_t* sym = &symtab[sym_idx];
            const char* name =
                elf64_get_string_from_dynstr(&image->u.elf.elf, sym->st_name);
            if (name == NULL)
                OE_RAISE(OE_NOT_FOUND);

            OE_TRACE_INFO("Linking symbol %s with idx  %lu", name, sym_idx);

            // Find the definition of the symbol in this image.
            oe_enclave_image_t* symbol_image = NULL;
            elf64_sym_t sym_defn = {0};
            if (elf64_find_dynamic_symbol_by_name(
                    &image->u.elf.elf, name, &sym_defn) == 0 &&
                sym_defn.st_value)
            {
                symbol_image = image;
            }
            else
            {
                // Search for the symbol in other images.
                for (uint64_t j = 0; j < num_deps; ++j)
                {
                    if (elf64_find_dynamic_symbol_by_name(
                            &deps[j]->u.elf.elf, name, &sym_defn) == 0 &&
                        sym_defn.st_value)
                    {
                        symbol_image = deps[j];
                        break;
                    }
                }
            }

            if (symbol_image)
            {
                p->r_addend = (int64_t)(
                    symbol_image->image_rva + sym_defn.st_value -
                    image->image_rva);
                OE_TRACE_INFO(
                    "%s vaddr=0x%lx, r_addend = 0xlx",
                    sym_defn.st_value,
                    p->r_addend);
            }
            else
            {
                OE_TRACE_INFO("symbol %s not found in secondary image\n", name);
            }
        }
    }

    result = OE_OK;
done:
    return result;
}

static oe_result_t _program_link(oe_enclave_image_t* image)
{
    oe_result_t result = OE_UNEXPECTED;
    if (image->secondary_image)
    {
        OE_TRACE_INFO("Performing program linking\n");

        oe_enclave_image_t* deps[] = {image->secondary_image};
        OE_CHECK(_link_symbols(image, deps, 1));

        // Process secondary images.
        OE_CHECK(_link_symbols(deps[0], deps, 1));
    }

    result = OE_OK;
done:
    return result;
}

static oe_result_t _patch(
    oe_enclave_image_t* image,
    size_t enclave_size,
    oe_enclave_t* enclave)
{
    oe_result_t result = OE_UNEXPECTED;
    oe_sgx_enclave_properties_t* oeprops;
    size_t i;
    uint64_t enclave_rva = 0;
    uint64_t aligned_size = 0;
    oe_module_link_info_t* link_info;
    size_t linked_modules_rva = 0;

    oeprops =
        (oe_sgx_enclave_properties_t*)(image->image_base + image->oeinfo_rva);

    assert((image->image_size & (OE_PAGE_SIZE - 1)) == 0);
    assert((image->reloc_size & (OE_PAGE_SIZE - 1)) == 0);
    assert((enclave_size & (OE_PAGE_SIZE - 1)) == 0);

    OE_CHECK(_get_dynamic_symbol_rva(
        image, "oe_linked_modules", &linked_modules_rva));
    link_info =
        (oe_module_link_info_t*)(image->image_base + linked_modules_rva);

    /* Clear certain ELF header fields */
    for (i = 0; i < image->u.elf.num_segments; i++)
    {
        const oe_elf_segment_t* seg = &image->u.elf.segments[i];
        elf64_ehdr_t* ehdr = (elf64_ehdr_t*)(image->image_base + seg->vaddr);

        if (elf64_test_header(ehdr) == 0)
        {
            ehdr->e_shoff = 0;
            ehdr->e_shnum = 0;
            ehdr->e_shstrndx = 0;
            break;
        }
    }
    if (image->secondary_image)
    {
        oe_enclave_image_t* simage = image->secondary_image;
        for (size_t j = 0; j < simage->u.elf.num_segments; j++)
        {
            const oe_elf_segment_t* seg = &simage->u.elf.segments[j];
            elf64_ehdr_t* ehdr =
                (elf64_ehdr_t*)(simage->image_base + seg->vaddr);

            if (elf64_test_header(ehdr) == 0)
            {
                ehdr->e_shoff = 0;
                ehdr->e_shnum = 0;
                ehdr->e_shstrndx = 0;
                break;
            }
        }

        uint64_t module_address = enclave->addr;
        module_address += image->image_size + image->reloc_size;

        // Convert virtual addresses to actuall address.
        enclave->debug_modules[0].base_address = module_address;
        enclave->debug_modules[0].size =
            simage->image_size + simage->reloc_size;

        uint64_t module_rva = image->image_size + image->reloc_size;
        simage->image_rva = module_rva;
        link_info[0].base_rva = module_rva;
        link_info[0].reloc_rva = module_rva + simage->image_size;
        link_info[0].reloc_size = simage->reloc_size;
        link_info[0].tdata_rva = module_rva + simage->tdata_rva;
        link_info[0].tdata_size = module_rva + simage->tdata_size;
        link_info[0].tdata_align = module_rva + simage->tdata_align;
        link_info[0].tbss_size = module_rva + simage->tbss_size;
        link_info[0].tbss_align = module_rva + simage->tbss_align;
        link_info[0].entry_rva = module_rva + simage->entry_rva;

        elf64_shdr_t init_section = {0};
        if (elf64_find_section_header(
                &simage->u.elf.elf, ".init_array", &init_section) == 0)
        {
            link_info[0].init_array_rva = module_rva + init_section.sh_addr;
            link_info[0].init_array_size = init_section.sh_size;
        }
        elf64_shdr_t fini_section = {0};
        if (elf64_find_section_header(
                &simage->u.elf.elf, ".fini_array", &fini_section) == 0)
        {
            link_info[0].fini_array_rva = module_rva + fini_section.sh_addr;
            link_info[0].fini_array_size = fini_section.sh_size;
        }
    }

    oeprops->image_info.enclave_size = enclave_size;
    oeprops->image_info.oeinfo_rva = image->oeinfo_rva;
    oeprops->image_info.oeinfo_size = sizeof(oe_sgx_enclave_properties_t);

    /* Set _enclave_rva to its own rva offset*/
    OE_CHECK(_get_dynamic_symbol_rva(image, "_enclave_rva", &enclave_rva));
    OE_CHECK(
        _set_uint64_t_dynamic_symbol_value(image, "_enclave_rva", enclave_rva));

    /* reloc right after image */
    oeprops->image_info.reloc_rva = image->image_size;
    oeprops->image_info.reloc_size = image->reloc_size;
    OE_CHECK(_set_uint64_t_dynamic_symbol_value(
        image, "_reloc_rva", image->image_size));
    OE_CHECK(_set_uint64_t_dynamic_symbol_value(
        image, "_reloc_size", image->reloc_size));

    /* heap right after image */
    oeprops->image_info.heap_rva = image->image_size + image->reloc_size;
    if (image->secondary_image)
        oeprops->image_info.heap_rva += image->secondary_image->image_size +
                                        image->secondary_image->reloc_size;

    if (image->tdata_size)
    {
        _set_uint64_t_dynamic_symbol_value(
            image, "_tdata_rva", image->tdata_rva);
        _set_uint64_t_dynamic_symbol_value(
            image, "_tdata_size", image->tdata_size);
        _set_uint64_t_dynamic_symbol_value(
            image, "_tdata_align", image->tdata_align);

        aligned_size +=
            oe_round_up_to_multiple(image->tdata_size, image->tdata_align);
    }
    if (image->tbss_size)
    {
        _set_uint64_t_dynamic_symbol_value(
            image, "_tbss_size", image->tbss_size);
        _set_uint64_t_dynamic_symbol_value(
            image, "_tbss_align", image->tbss_align);

        aligned_size +=
            oe_round_up_to_multiple(image->tbss_size, image->tbss_size);
    }

    if (aligned_size > OE_THREAD_LOCAL_SPACE)
    {
        OE_TRACE_ERROR(
            "Thread-local variables exceed available thread-local space.\n");
        OE_RAISE(OE_FAILURE);
    }

    /* Clear the hash when taking the measure */
    memset(oeprops->sigstruct, 0, sizeof(oeprops->sigstruct));

    /* Perform programing linking */
    OE_CHECK(_program_link(image));

    result = OE_OK;
done:
    return result;
}

static oe_result_t _sgx_load_enclave_properties(
    const oe_enclave_image_t* image,
    const char* section_name,
    oe_sgx_enclave_properties_t* properties)
{
    oe_result_t result = OE_UNEXPECTED;
    OE_UNUSED(section_name);

    /* Copy from the image at oeinfo_rva. */
    OE_CHECK(oe_memcpy_s(
        properties,
        sizeof(*properties),
        image->image_base + image->oeinfo_rva,
        sizeof(*properties)));

    result = OE_OK;

done:
    return result;
}

static oe_result_t _sgx_update_enclave_properties(
    const oe_enclave_image_t* image,
    const char* section_name,
    const oe_sgx_enclave_properties_t* properties)
{
    oe_result_t result = OE_UNEXPECTED;
    OE_UNUSED(section_name);

    /* Copy to both the image and ELF file*/
    OE_CHECK(oe_memcpy_s(
        (uint8_t*)image->u.elf.elf.data + image->oeinfo_file_pos,
        sizeof(*properties),
        properties,
        sizeof(*properties)));

    OE_CHECK(oe_memcpy_s(
        image->image_base + image->oeinfo_rva,
        sizeof(*properties),
        properties,
        sizeof(*properties)));

    result = OE_OK;

done:
    return result;
}

static oe_result_t _load_secondary_modules(
    const char* path,
    oe_enclave_image_t* image,
    oe_enclave_t* enclave)
{
    oe_result_t result = OE_UNEXPECTED;
    char* module_name = NULL;
    size_t module_name_len = 0;
    char* secondary_image_path = NULL;
    oe_enclave_image_t* secondary_image = NULL;

    if (elf64_find_section(
            &image->u.elf.elf,
            "oemoduls",
            (unsigned char**)&module_name,
            &module_name_len) == 0)
    {
        OE_TRACE_VERBOSE("Enclave needs secondary modules");
        OE_TRACE_INFO("module name = %s\n", module_name);

        // Find out the folder from enclave path.
        const char* p = path + strlen(path);
        while (p != path && *p != '/' && *p != '\\')
            --p;

        // Allocate string to hold the seconday enclave path.
        size_t len = (size_t)(p - path) + strlen(module_name) + 1;
        secondary_image_path = calloc(1, len);
        if (!secondary_image_path)
            OE_RAISE(OE_OUT_OF_MEMORY);

        sprintf(
            secondary_image_path,
            "%.*s/%s",
            (int)(p - path),
            path,
            module_name);
        OE_TRACE_INFO("module path = %s\n", secondary_image_path);
        secondary_image =
            (oe_enclave_image_t*)calloc(1, sizeof(*secondary_image));
        if (!secondary_image)
            OE_RAISE(OE_OUT_OF_MEMORY);

        OE_CHECK(_load_elf_image(
            secondary_image_path, secondary_image, 0 /* secondary image*/));

        enclave->debug_modules =
            (oe_debug_module_t*)calloc(1, sizeof(oe_debug_module_t));
        if (!enclave->debug_modules)
            OE_RAISE(OE_OUT_OF_MEMORY);
        enclave->num_debug_modules = 1;

        enclave->debug_modules[0].magic = OE_DEBUG_MODULE_MAGIC;
        enclave->debug_modules[0].version = 1;
        enclave->debug_modules[0].path = secondary_image_path;
        enclave->debug_modules[0].path_length = strlen(secondary_image_path);

        image->secondary_image = secondary_image;
        secondary_image = NULL;
        secondary_image_path = NULL;
    }
    else
    {
        OE_TRACE_VERBOSE("Enclave does not specify secondary modules.");
    }

    result = OE_OK;
done:
    if (secondary_image_path)
        free(secondary_image_path);
    if (secondary_image)
        free(secondary_image);

    return result;
}

oe_result_t oe_load_elf_enclave_image(
    const char* path,
    oe_enclave_image_t* image,
    oe_enclave_t* enclave)
{
    oe_result_t result = OE_UNEXPECTED;

    memset(image, 0, sizeof(oe_enclave_image_t));

    /* Load the program segments into memory */
    OE_CHECK(_load_elf_image(path, image, 1 /* primary image */));

    /* Load secondary modules into memory */
    OE_CHECK(_load_secondary_modules(path, image, enclave));

    /* Load the relocations into memory (zero-padded to next page size) */
    if (elf64_load_relocations(
            &image->u.elf.elf, &image->u.elf.reloc_data, &image->reloc_size) !=
        0)
        OE_RAISE(OE_FAILURE);

    /* Load the relocations of secondary image */
    if (image->secondary_image)
    {
        if (elf64_load_relocations(
                &image->secondary_image->u.elf.elf,
                &image->secondary_image->u.elf.reloc_data,
                &image->secondary_image->reloc_size) != 0)
            OE_RAISE(OE_FAILURE);
    }

    if (oe_get_current_logging_level() >= OE_LOG_LEVEL_VERBOSE)
        _dump_relocations(image->u.elf.reloc_data, image->reloc_size);

    image->type = OE_IMAGE_TYPE_ELF;
    image->calculate_size = _calculate_size;
    image->add_pages = _add_pages;
    image->patch = _patch;
    image->sgx_load_enclave_properties = _sgx_load_enclave_properties;
    image->sgx_update_enclave_properties = _sgx_update_enclave_properties;
    image->unload = _unload;

    result = OE_OK;

done:

    if (OE_OK != result)
        _unload(image);

    return result;
}
