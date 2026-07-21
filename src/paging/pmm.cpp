#include "pmm.h"

static void* memset(void* ptr, int value, size_t size) {

	uint8_t* p = (uint8_t*)ptr;
    	
	for (size_t i = 0; i < size; i++) {
    	
		p[i] = (uint8_t)value;
    	
	}
    	
	return ptr;

}

void PMM::init(limine_memmap_response* memmap, uint64_t hhdm) {
    bitmap = nullptr;
    uint64_t highest = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        limine_memmap_entry* entry = memmap->entries[i];
        uint64_t end = entry->base + entry->length;
        if (end > highest) highest = end;
    }
    frame_count = highest / 4096;
    size_t bit_count = frame_count / 8;

    uint64_t bitmap_phys = 0;   // NEU: physische Adresse merken
    for (size_t i = 0; i < memmap->entry_count; i++) {
        limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == 0 && entry->length >= bit_count) {
            bitmap_phys = entry->base;              // NEU
            bitmap = (uint64_t*)(entry->base + hhdm);
            break;
        }
    }
    if (bitmap == nullptr) { while (true) asm volatile("hlt"); }

    memset(bitmap, 0xFF, bit_count);

    for (size_t i = 0; i < memmap->entry_count; i++) {
        limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == 0) {
            size_t start = entry->base / 4096;
            size_t count = entry->length / 4096;
            for (size_t j = 0; j < count; j++) clear_bit(start + j);
        }
    }

    // NEU: Frames, in denen die Bitmap selbst liegt, wieder als belegt markieren
    size_t bitmap_start_frame = bitmap_phys / 4096;
    size_t bitmap_frame_count = (bit_count + 4095) / 4096;
    for (size_t j = 0; j < bitmap_frame_count; j++) {
        set_bit(bitmap_start_frame + j);
    }

    hhdm_offset = hhdm;
}

void PMM::set_bit(size_t index) {
	
	size_t block = index / 64;
	size_t bit = index % 64;

	bitmap[block] |= (1ULL << bit);

}

void PMM::clear_bit(size_t index) {

	size_t block = index / 64;
	size_t bit = index % 64;

	bitmap[block] &= ~(1ULL << bit);

}

bool PMM::test_bit(size_t index) {

	size_t block = index / 64;
	size_t bit = index % 64;

	return (bitmap[block] & (1ULL << bit)) != 0;

}

void* PMM::alloc_frame() {
    for (size_t i = 0; i < frame_count; i++) {
        if (!test_bit(i)) {
            set_bit(i);
	    void* addr = (void*)(i * 4096 + hhdm_offset);
            memset(addr, 0, 4096);
            return (void*)(i * 4096);
        }
    }
    return nullptr;
}

void PMM::free_frame(void* addr) {
    size_t index = (uint64_t)addr / 4096;
    if (!test_bit(index)) {
	    while (true) asm volatile("hlt");
    }
    clear_bit(index);
}

void* PMM::alloc_frames(size_t count) {
    size_t run_start = 0;
    size_t run_length = 0;

    for (size_t i = 0; i < frame_count; i++) {
        if (!test_bit(i)) {
            if (run_length == 0) run_start = i;
            run_length++;
            if (run_length == count) {
                // gefunden -> alle count Bits setzen
                for (size_t j = 0; j < count; j++) {
                    set_bit(run_start + j);
                }
                void* addr = (void*)(run_start * 4096 + hhdm_offset);
                memset(addr, 0, count * 4096);
                return (void*)(run_start * 4096);
            }
        } else {
            run_length = 0; // Kette unterbrochen, neu anfangen
        }
    }
    return nullptr; // kein zusammenhängender Block gefunden
}
