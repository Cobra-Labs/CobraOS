#include "elf.h"

static bool is_valid_elf(const uint8_t* data) {
    return data[0] == 0x7F &&
           data[1] == 'E'  &&
           data[2] == 'L'  &&
           data[3] == 'F';
}

uint64_t elf_load(VMM* vmm, PMM* pmm, PageTable* target, const uint8_t* data, uint64_t size) {
    if (!is_valid_elf(data)) return 0;

    const Elf64_Ehdr* ehdr = (const Elf64_Ehdr*)data;
    if (ehdr->e_machine != 0x3E) return 0; // x86_64

    for (uint16_t i = 0; i < ehdr->e_phnum; i++) {
        const Elf64_Phdr* ph = (const Elf64_Phdr*)(data + ehdr->e_phoff + i * ehdr->e_phentsize);

        if (ph->p_type != PT_LOAD) continue;

        uint64_t start = ph->p_vaddr & ~0xFFFULL;
        uint64_t end   = (ph->p_vaddr + ph->p_memsz + 0xFFF) & ~0xFFFULL;

        for (uint64_t page = start; page < end; page += 0x1000) {
            uint64_t frame = (uint64_t)pmm->alloc_frame();
            uint8_t* dst = (uint8_t*)(frame + pmm->get_hhdm());

            for (int j = 0; j < 0x1000; j++) dst[j] = 0;

            uint64_t file_start = ph->p_vaddr;
            uint64_t file_end   = ph->p_vaddr + ph->p_filesz;

            uint64_t copy_start = (page > file_start) ? page : file_start;
            uint64_t copy_end   = (page + 0x1000 < file_end) ? page + 0x1000 : file_end;

            if (copy_start < copy_end) {
                uint64_t page_offset = copy_start - page;
                uint64_t file_offset = ph->p_offset + (copy_start - ph->p_vaddr);
                uint64_t len = copy_end - copy_start;

                for (uint64_t k = 0; k < len; k++) {
                    dst[page_offset + k] = data[file_offset + k];
                }
            }

            uint64_t flags = PAGE_USER;
            if (ph->p_flags & PF_W) flags |= PAGE_WRITE;
            vmm->map_page_in(target, page, frame, flags);
        }
    }

    return ehdr->e_entry;
}
