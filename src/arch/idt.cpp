#include "idt.h"
#include "../paging/vmm.h"

uint64_t g_hhdm_offset = 0;

struct InterruptFrame {
	uint64_t ip;
	uint64_t cs;
	uint64_t flags;
	uint64_t sp;
	uint64_t ss;
};

__attribute__((interrupt))
void default_handler([[maybe_unused]] InterruptFrame* frame) {

}

__attribute__((interrupt))
void page_fault_handler(InterruptFrame* frame, uint64_t error_code) {
    uint64_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));
    qemu_print("PAGE FAULT! CR2: ");
    qemu_print_hex(cr2);
    qemu_print(" error: ");
    qemu_print_hex(error_code);
    qemu_print("\n");
    while (true) asm volatile("hlt");
}

__attribute__((interrupt))
void timer_handler([[maybe_unused]] InterruptFrame* frame) {
    *((volatile uint32_t*)(g_hhdm_offset + 0xFEE00000 + 0x0B0)) = 0;
}

__attribute__((interrupt))
void gpf_handler(InterruptFrame* frame, uint64_t error_code) {
    qemu_print("GPF! error: ");
    qemu_print_hex(error_code);
    qemu_print("\n");
    while (true) asm volatile("hlt");
}

__attribute__((interrupt))
void double_fault_handler(InterruptFrame* frame, uint64_t error_code) {
    qemu_print("DOUBLE FAULT!\n");
    while (true) asm volatile("hlt");
}

__attribute__((interrupt))
void invalid_opcode_handler(InterruptFrame* frame) {
    qemu_print("INVALID OPCODE!\n");
    while (true) asm volatile("hlt");
}

void IDTManager::set_gate(int index, uint64_t handler, uint8_t flags) {
	entries[index].offset_low  = handler & 0xFFFF;
    	entries[index].selector    = 0x08;  // Kernel Code Segment
    	entries[index].ist         = 0;
    	entries[index].flags       = flags;
    	entries[index].offset_mid  = (handler >> 16) & 0xFFFF;
    	entries[index].offset_high = (handler >> 32) & 0xFFFFFFFF;
    	entries[index].reserved    = 0;
}

void IDTManager::init() {
	idtr.limit = sizeof(entries) - 1;
	idtr.base = (uint64_t)&entries;

	asm volatile ("lidt %0" :: "m"(idtr));

	for (int i = 0; i < 256; i++) {
    		set_gate(i, (uint64_t)default_handler, 0x8E);
	}
	
	// Page Fault ist Interrupt 14
	set_gate(14, (uint64_t)page_fault_handler, 0x8E);
	set_gate(32, (uint64_t)timer_handler, 0x8E);
    set_gate(8,  (uint64_t)double_fault_handler, 0x8E);
    set_gate(13, (uint64_t)gpf_handler, 0x8E);
    set_gate(6, (uint64_t)invalid_opcode_handler, 0x8E);
}
