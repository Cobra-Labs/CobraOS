#include "unistd.h"
#include "syscall.h"

extern "C" long write(int fd, const void* buf, size_t count) {
    (void)fd;  // Kernel kennt aktuell kein fd, ignoriert es komplett
    return __syscall2(SYS_WRITE, (long)buf, (long)count);
}

extern "C" [[noreturn]] void _exit(int code) {
    (void)code;
    __syscall0(SYS_EXIT);
    __builtin_unreachable();
}

extern "C" long getpid(void) {
    return __syscall0(SYS_GETPID);
}
