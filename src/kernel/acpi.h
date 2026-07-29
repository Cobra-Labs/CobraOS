#include <stdint.h>
#include <stddef.h>
#include "kutil.h"

#pragma once

struct __attribute__((packed)) Rsdp {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;

    uint32_t length;
    uint64_t xdst_address;
    uint8_t extended_checksum;
    uint8_t reserved[8];
};

class Acpi {
    public:
        void init(void* rsdp_virt, uint64_t hhdm_offset);
    private:
        bool validate_checksum(const void* table, size_t length);
        bool use_xsdt;
        void* rsdt_or_xsdt;
};
