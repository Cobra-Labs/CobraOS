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

void ProcessManager::start(uint64_t entry_point, uint64_t stack_pointer) {
    void* stack_frame = pmm->alloc_frame();
    uint64_t stack_phys = (uint64_t)stack_frame;
    uint64_t stack_virt = 0x500000;

    vmm->map_page(stack_virt, stack_phys,
                  PAGE_PRESENT | PAGE_WRITE | PAGE_USER | PAGE_NX);

    uint64_t cur_rsp;
    asm volatile("mov %%rsp, %0" : "=r"(cur_rsp));
    vmm->map_page(0x600000, (uint64_t)pmm->alloc_frame(), PAGE_PRESENT | PAGE_WRITE | PAGE_USER | PAGE_NX);
    do_iretq(entry_point, stack_pointer);
}

void ProcessManager::kill(uint64_t pid) {
    // TODO: Prozess beenden
}
