#pragma once
#include <stdint.h>
#include <stddef.h>

constexpr uint32_t MSR_EFER  = 0xC0000080;
constexpr uint32_t MSR_STAR  = 0xC0000081;
constexpr uint32_t MSR_LSTAR = 0xC0000082;
constexpr uint32_t MSR_GSBASE        = 0xC0000101;
constexpr uint32_t MSR_KERNEL_GSBASE = 0xC0000102;

struct CPULocal {
    uint64_t kernel_stack;  // Kernel-Stack für Syscalls
    uint64_t user_stack;    // User-Stack temporär gespeichert
};

class SyscallManager {
	public:
		void init();
};

inline void wrmsr(uint32_t msr, uint64_t value) {
    	uint32_t low  = value & 0xFFFFFFFF;
    	uint32_t high = value >> 32;
    	asm volatile("wrmsr" :: "c"(msr), "a"(low), "d"(high));
}

inline uint64_t rdmsr(uint32_t msr) {
    	uint32_t low, high;
    	asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    	return ((uint64_t)high << 32) | low;
}

extern CPULocal g_cpu_local;
