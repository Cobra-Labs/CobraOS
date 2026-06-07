#include "syscall.h"
#include "../paging/pmm.h"
#include "../paging/vmm.h"

extern PMM* volatile g_pmm;
extern VMM* volatile g_vmm;

constexpr uint64_t SYS_EXIT   = 0;
constexpr uint64_t SYS_WRITE  = 1;
constexpr uint64_t SYS_GETPID = 2;
constexpr uint64_t SYS_MMAP   = 3;

CPULocal g_cpu_local = {};

__attribute__((naked))
void syscall_handler() {
    asm volatile(
        "swapgs\n"
        "mov %rsp, (g_cpu_local + 8)\n"  // user_stack
        "mov (g_cpu_local), %rsp\n"       // kernel_stack
        "push %rcx\n"
        "push %r11\n"
        "mov %rdx, %rcx\n"
        "mov %rsi, %rdx\n"
        "mov %rdi, %rsi\n"
        "mov %rax, %rdi\n"
        "call syscall_dispatch\n"
        "pop %r11\n"
        "pop %rcx\n"
        "mov (g_cpu_local + 8), %rsp\n"
        "swapgs\n"
        "sysretq\n"
    );
}

void SyscallManager::init() {
	wrmsr(MSR_EFER, rdmsr(MSR_EFER) | 1);
    wrmsr(MSR_STAR, ((uint64_t)0x08 << 32) | ((uint64_t)0x18 << 48));
	wrmsr(MSR_LSTAR, (uint64_t)syscall_handler);
    wrmsr(MSR_KERNEL_GSBASE, (uint64_t)&g_cpu_local);
    wrmsr(MSR_GSBASE, 0);
    wrmsr(MSR_KERNEL_GSBASE, (uint64_t)&g_cpu_local);
}

extern "C" uint64_t syscall_dispatch(uint64_t nr, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    PMM* pmm = (PMM*)g_pmm;
    VMM* vmm = (VMM*)g_vmm;

    switch (nr) {
        case SYS_EXIT:
            while (true) asm volatile("hlt");
            return 0;

        case SYS_WRITE: {
            const char* str = (const char*)arg1;
            for (uint64_t i = 0; i < arg2; i++)
                asm volatile("outb %0, %1" :: "a"((uint8_t)str[i]), "Nd"((uint16_t)0xE9));
            return arg2;
        }

        case SYS_GETPID:
            return 1; // vorerst immer PID 1

        case SYS_MMAP: {
            // arg1 = virtuelle Adresse, arg2 = größe in bytes
            uint64_t virt = arg1;
            uint64_t size = arg2;
            for (uint64_t off = 0; off < size; off += 4096) {
                void* frame = g_pmm->alloc_frame();
                g_vmm->map_page(virt + off, (uint64_t)frame,
                    PAGE_PRESENT | PAGE_WRITE | PAGE_USER | PAGE_NX);
            }
            return arg1; // gibt die Adresse zurück
        }

        default:
            return (uint64_t)-1;
    }
}
