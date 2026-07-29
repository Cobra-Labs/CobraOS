#include "acpi.h"

bool Acpi::validate_checksum(const void* table, size_t length) {
    const uint8_t* bytes = (const uint8_t*)table;
    uint8_t sum = 0;

    for (size_t i = 0; i < length; i++) {
        sum += bytes[i];
    }

    return sum == 0;
}

void Acpi::init(void* rsdp_virt, uint64_t hhdm_offset) {
    Rsdp* rsdp = (Rsdp*)rsdp_virt;

    if (memcmp(rsdp->signature, "RSD PTR ", 8) != 0) { /* panic */ }
    if (!validate_checksum(rsdp, 20)) { /* panic */ }

    if (rsdp->revision >= 2) {
        if (!validate_checksum(rsdp, rsdp->length)) { /* panic */ }
        use_xsdt = true;
        rsdt_or_xsdt = (void*)(rsdp->xdst_address + hhdm_offset);
    } else {
        use_xsdt = false;
        rsdt_or_xsdt = (void*)((uint64_t)rsdp->rsdt_address + hhdm_offset);
    }
}
