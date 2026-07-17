#pragma once
#include <stdint.h>
#include <stddef.h>
#include "pmm.h"

struct PageTable {
    	
	uint64_t entries[512];

} __attribute__((aligned(4096)));

constexpr uint64_t PAGE_PRESENT = (1ULL << 0);
constexpr uint64_t PAGE_WRITE   = (1ULL << 1);
constexpr uint64_t PAGE_USER    = (1ULL << 2);
constexpr uint64_t PAGE_NX      = (1ULL << 63);

inline uint64_t pml4_idx(uint64_t virt) { return (virt >> 39) & 0x1FF; }
inline uint64_t pdpt_idx(uint64_t virt) { return (virt >> 30) & 0x1FF; }
inline uint64_t pd_idx  (uint64_t virt) { return (virt >> 21) & 0x1FF; }
inline uint64_t pt_idx  (uint64_t virt) { return (virt >> 12) & 0x1FF; }
inline void load_cr3(uint64_t phys) {
    asm volatile("mov %0, %%cr3" :: "r"(phys) : "memory");
}


class VMM {
	
	public:

		void init(PMM* pmm);
		void map_page(uint64_t virt, uint64_t phys, uint64_t flags);
        void map_page_in(PageTable* target, uint64_t virt, uint64_t phys, uint64_t flags);
        PageTable* create_address_space();
        uint64_t get_phys(PageTable* table);
		void activate();
	
	private:
		
		PageTable* pml4;
		PMM* pmm;

};
