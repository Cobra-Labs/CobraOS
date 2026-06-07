#pragma once
#include <stdint.h>
#include <stddef.h>

constexpr uint64_t LAPIC_BASE        = 0xFEE00000;

constexpr uint32_t LAPIC_ID          = 0x020;  // LAPIC ID
constexpr uint32_t LAPIC_EOI         = 0x0B0;  // End of Interrupt
constexpr uint32_t LAPIC_SPURIOUS    = 0x0F0;  // Spurious Interrupt Vector
constexpr uint32_t LAPIC_TIMER_LVT   = 0x320;  // Timer Local Vector Table
constexpr uint32_t LAPIC_TIMER_INIT  = 0x380;  // Timer Initial Count
constexpr uint32_t LAPIC_TIMER_DIV   = 0x3E0;  // Timer Divide Config

inline void lapic_write(uint64_t base, uint32_t reg, uint32_t val) {
    *((volatile uint32_t*)(base + reg)) = val;
}
inline uint32_t lapic_read(uint64_t base, uint32_t reg) {
    return *((volatile uint32_t*)(base + reg));
}

class APICManager {
	public:
    		void init(uint64_t hddm);

	private:	
    		void enable();
    		void init_timer();
		uint64_t hhdm_offset;
};
