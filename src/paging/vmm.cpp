#include "pmm.h"
#include "vmm.h"

void VMM::init(PMM* p) {

	pmm = p;
	uint64_t phys = (uint64_t)pmm->alloc_frame();
	pml4 = (PageTable*)(phys + pmm->get_hhdm());

}

void VMM::map_page(uint64_t virt, uint64_t phys, uint64_t flags) {

    	uint64_t i4 = pml4_idx(virt);
    	uint64_t i3 = pdpt_idx(virt);
    	uint64_t i2 = pd_idx(virt);
    	uint64_t i1 = pt_idx(virt);

    	// Ebene 1: PML4 → PDPT
    	if (!(pml4->entries[i4] & PAGE_PRESENT)) {

		uint64_t f = (uint64_t)pmm->alloc_frame();
		pml4->entries[i4] = f | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;;

    	}

    	PageTable* pdpt = (PageTable*)((pml4->entries[i4] & ~0xFFFULL) + pmm->get_hhdm());

    	// Ebene 2: PDPT → PD
    	if (!(pdpt->entries[i3] & PAGE_PRESENT)) {

		uint64_t f = (uint64_t)pmm->alloc_frame();
	    	pdpt->entries[i3] = f | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;;

    	}

	PageTable* pd = (PageTable*)((pdpt->entries[i3] & ~0xFFFULL) + pmm->get_hhdm());

    	// Ebene 3: PD → PT

    	if (!(pd->entries[i2] & PAGE_PRESENT)) {

		uint64_t f = (uint64_t)pmm->alloc_frame();
	    	pd->entries[i2] = f | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;

    	}

	PageTable* pt = (PageTable*)((pd->entries[i2] & ~0xFFFULL) + pmm->get_hhdm());

    	// Ebene 4: PT → physische Page eintragen
    	pt->entries[i1] = phys | flags | PAGE_PRESENT;

}

void VMM::activate() {
    uint64_t phys = (uint64_t)pml4 - pmm->get_hhdm();
    load_cr3(phys);
}
