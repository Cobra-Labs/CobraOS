#include "apic.h"
#include "idt.h"

void APICManager::enable() {
	uint64_t base = LAPIC_BASE + hhdm_offset;
	lapic_write(base, LAPIC_SPURIOUS, lapic_read(base, LAPIC_SPURIOUS) | 0x1FF);
}

void APICManager::init_timer() {
    	uint64_t base = LAPIC_BASE + hhdm_offset;
	lapic_write(base, LAPIC_TIMER_DIV, 0x3);
	lapic_write(base, LAPIC_TIMER_LVT, 0x20020);
	lapic_write(base, LAPIC_TIMER_INIT, 10000000);
}
	
void APICManager::init(uint64_t hhdm) {
    	hhdm_offset = hhdm;
    	enable();
    	init_timer();
}
