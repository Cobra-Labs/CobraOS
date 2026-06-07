// Syscall-Nummern
#define SYS_EXIT  0
#define SYS_WRITE 1

static void sys_write(const char* str, unsigned long len) {
    __asm__ volatile(
        "syscall"
        :: "a"(SYS_WRITE), "D"(str), "S"(len)
        : "rcx", "r11", "memory"
    );
}

static void sys_exit() {
    __asm__ volatile(
        "syscall"
        :: "a"(SYS_EXIT)
    );
}

void _start() {
    sys_write("Hello from ELF!\n", 16);
    sys_exit();
}
