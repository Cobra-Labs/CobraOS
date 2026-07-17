// libc/include/syscall.h
#pragma once

#define SYS_EXIT   0
#define SYS_WRITE  1
#define SYS_GETPID 2
#define SYS_MMAP   3

extern "C" {
    long __syscall0(long nr);
    long __syscall1(long nr, long a1);
    long __syscall2(long nr, long a1, long a2);
    long __syscall3(long nr, long a1, long a2, long a3);
}
