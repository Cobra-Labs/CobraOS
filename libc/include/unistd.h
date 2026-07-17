// unistd.h
#pragma once
#include <stddef.h>

extern "C" {
    long write(int fd, const void* buf, size_t count);
    [[noreturn]] void _exit(int code);
    long getpid(void);
}
