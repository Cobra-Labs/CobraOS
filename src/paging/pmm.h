#pragma once
#include <stdint.h>
#include <stddef.h>

struct limine_memmap_entry {
    uint64_t base;
    uint64_t length;
    uint64_t type;
};

struct limine_memmap_response {
    uint64_t revision;
    uint64_t entry_count;
    limine_memmap_entry** entries;
};

struct limine_memmap_request {
    uint64_t id[4];
    uint64_t revision;
    limine_memmap_response* response;
};

struct limine_hhdm_response {
    uint64_t revision;
    uint64_t offset;
};

struct limine_hhdm_request {
    uint64_t id[4];
    uint64_t revision;
    limine_hhdm_response* response;
};

inline void qemu_print(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++)
        asm volatile("outb %0, %1" : : "a"((uint8_t)str[i]), "Nd"((uint16_t)0xE9));
}

inline void qemu_print_hex(uint64_t val) {
    const char* hex = "0123456789ABCDEF";
    char buf[19];
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 0; i < 16; i++)
        buf[2 + i] = hex[(val >> (60 - i * 4)) & 0xF];
    buf[18] = '\0';
    qemu_print(buf);
}

class PMM {
public:
    void  init(limine_memmap_response* memmap, uint64_t hhdm);
    void* alloc_frame();
    void  free_frame(void* addr);
    uint64_t get_hhdm() { return hhdm_offset; }

private:
    void set_bit(size_t index);
    void clear_bit(size_t index);
    bool test_bit(size_t index);

    uint64_t* bitmap;
    size_t    frame_count;
    uint64_t  hhdm_offset;
};
