#pragma once
#include <stdint.h>
#include <stddef.h>

class GDTManager;

struct TSS {
	uint32_t reserved0;
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint64_t reserved1;
	uint64_t ist[7];
	uint64_t reserved2;
	uint16_t reserved3;
	uint16_t iopb;
} __attribute__((packed));

struct TSSDescriptor {
    	uint16_t limit_low;
    	uint16_t base_low;
    	uint8_t  base_mid;
    	uint8_t  flags1;
    	uint8_t  flags2;
    	uint8_t  base_high;
    	uint32_t base_upper;
    	uint32_t reserved;
} __attribute__((packed));

class TSSManager {
	public:
		void init(uint64_t kernel_stack, GDTManager* gdt);
	
	private:
		TSS tss;
};


