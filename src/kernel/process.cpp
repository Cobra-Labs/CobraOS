#include "process.h"

void ProcessManager::init(PMM* p, VMM* v) {
    pmm = p;
    vmm = v;
}

__attribute__((naked))
static void do_iretq(uint64_t entry, uint64_t stack) {
    asm volatile(
        "sub $40, %rsp\n"
        "movq $0x23, 32(%rsp)\n"
        "movq %rsi,  24(%rsp)\n"
        "movq $0x202, 16(%rsp)\n"
        "movq $0x2B,  8(%rsp)\n"
        "movq %rdi,   0(%rsp)\n"
        "swapgs\n"
        "iretq\n"
        "ud2\n"
    );
}

void ProcessManager::start(PageTable* address_space, uint64_t entry_point, uint64_t stack_pointer) {
    // User-Stack-Seite im *neuen* Adressraum anlegen
    uint64_t stack_phys = (uint64_t)pmm->alloc_frame();
    uint64_t stack_page = (stack_pointer - 1) & ~0xFFFULL; // Seite, in der der Stack-Top liegt

    vmm->map_page_in(address_space, stack_page, stack_phys,
                      PAGE_PRESENT | PAGE_WRITE | PAGE_USER | PAGE_NX);

    uint64_t phys_pml4 = vmm->get_phys(address_space);
    load_cr3(phys_pml4);

    do_iretq(entry_point, stack_pointer);
}

void ProcessManager::kill(uint64_t pid) {
    // TODO: Prozess beenden
}
