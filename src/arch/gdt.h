#pragma once
#include <stdint.h>
#include <stddef.h>
#include "tss.h"

struct GDTEntry {
        uint16_t limit_low;    // ignoriert in 64-Bit
        uint16_t base_low;     // ignoriert in 64-Bit
        uint8_t  base_mid;     // ignoriert in 64-Bit
        uint8_t  access;       // wichtig — definiert Ring und Typ
        uint8_t  granularity;  // wichtig — definiert 64-Bit Modus
        uint8_t  base_high;    // ignoriert in 64-Bit
} __attribute__((packed));

struct GDT {
	GDTEntry entries[6];
	TSSDescriptor tss_desc;
} __attribute__((packed));

struct GDTR {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));

class GDTManager {
	public:
		void init();
		void set_tss(TSS* tss);

	private: 
		GDT gdt;
		GDTR gdtr;

		void set_entry(int index, uint8_t access, uint8_t granularity);
};
