#include "pci.h"
#include "io.h"

static uint32_t pci_address(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    return (1u << 31) | ((uint32_t)bus << 16) | ((uint32_t)slot << 11)
    | ((uint32_t)func << 8) | (offset & 0xFC);
}

uint32_t pci_config_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    outl(0xCF8, pci_address(bus, slot, func, offset));
    return inl(0xCFC);
}

void pci_config_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    outl(0xCF8, pci_address(bus, slot, func, offset));
    outl(0xCFC, value);
}

bool pci_find_device(uint16_t vendor_id, uint16_t device_id, PciDevice* out) {
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id = pci_config_read32(bus, slot, func, 0x00);
                uint16_t vid = id & 0xFFFF;
                if (vid == 0xFFFF) continue;

                uint16_t did = (id >> 16) & 0xFFFF;
                if (vid == vendor_id && did == device_id) {
                    out->bus = bus; out->slot = slot; out->func = func;
                    out->vendor_id = vid; out->device_id = did;

                    uint32_t class_reg = pci_config_read32(bus, slot, func, 0x08);
                    out->class_code = (class_reg >> 24) & 0xFF;
                    out->subclass   = (class_reg >> 16) & 0xFF;
                    out->prog_if    = (class_reg >> 8) & 0xFF;
                    return true;
                }
            }
        }
    }
    return false;
}
