#include "gdt.h"

void GDTManager::set_entry(int index, uint8_t access, uint8_t granularity) {
	gdt.entries[index].limit_low = 0;
	gdt.entries[index].base_low = 0;
	gdt.entries[index].base_mid = 0;
	gdt.entries[index].access = access;
	gdt.entries[index].granularity = granularity;
	gdt.entries[index].base_high = 0;
}

void GDTManager::init() {
	set_entry(0, 0x00, 0x00);
	set_entry(1, 0x9A, 0x20);
	set_entry(2, 0x92, 0x00);
    set_entry(3, 0x00, 0x00);
	set_entry(4, 0xF2, 0x00);
	set_entry(5, 0xFA, 0x20);

	gdtr.limit = sizeof(GDT) - 1;
	gdtr.base = (uint64_t)&gdt;

	asm volatile("lgdt %0" :: "m"(gdtr));

	asm volatile(
    	"mov $0x10, %%ax\n"  // 0x10 = Index 2 = Kernel Data Segment
    	"mov %%ax, %%ds\n"
    	"mov %%ax, %%es\n"
    	"mov %%ax, %%fs\n"
    	"mov %%ax, %%gs\n"
    	"mov %%ax, %%ss\n"
    	"push $0x08\n"        // 0x08 = Index 1 = Kernel Code Segment
    	"lea 1f(%%rip), %%rax\n"
    	"push %%rax\n"
    	"lretq\n"             // Far Return — lädt CS neu
    	"1:\n"
    	::: "rax", "memory"
	);
}

void GDTManager::set_tss(TSS* tss) {
	gdt.tss_desc.limit_low = sizeof(TSS) - 1;
	gdt.tss_desc.flags1    = 0x89;  // Present, Ring 0, TSS Typ
	gdt.tss_desc.flags2    = 0x00;
	gdt.tss_desc.reserved  = 0;

	uint64_t addr = (uint64_t)tss;

	gdt.tss_desc.base_low   =  addr        & 0xFFFF;
	gdt.tss_desc.base_mid   = (addr >> 16) & 0xFF;
	gdt.tss_desc.base_high  = (addr >> 24) & 0xFF;
	gdt.tss_desc.base_upper = (addr >> 32) & 0xFFFFFFFF;
	// GDTR neu laden weil GDT sich geändert hat
	asm volatile("lgdt %0" :: "m"(gdtr));

	// TSS laden — 0x28 = Index 5 in der GDT (6 * 8 = 40 = 0x30)
	asm volatile("ltr %0" :: "r"((uint16_t)0x30));
}
