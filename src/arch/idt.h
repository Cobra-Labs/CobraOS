#pragma once
#include <stdint.h>
#include <stddef.h>

extern uint64_t g_hhdm_offset;

struct IDTEntry {
        uint16_t offset_low;    // Bits 0-15 der Handler-Adresse
    	uint16_t selector;      // GDT Segment — immer 0x08 (Kernel Code)
    	uint8_t  ist;           // Interrupt Stack Table — vorerst 0
    	uint8_t  flags;         // Typ und Ring
    	uint16_t offset_mid;    // Bits 16-31 der Handler-Adresse
    	uint32_t offset_high;   // Bits 32-63 der Handler-Adresse
    	uint32_t reserved;      // immer 0
} __attribute__((packed));

struct IDTR {
    	uint16_t limit;
	uint64_t base;
} __attribute__((packed));

class IDTManager {
	public:
		void init();
		void set_gate(int index, uint64_t handler, uint8_t flags);

	private:
		IDTEntry entries[256];
		IDTR idtr;
};
