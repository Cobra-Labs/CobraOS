#include "virtio_blk.h"
#include "pci.h"
#include "io.h"

bool virtio_blk_init(VirtioBlkDevice* dev, PMM* pmm) {
    PciDevice blk_dev;
    if (!pci_find_device(0x1AF4, 0x1001, &blk_dev)) {
        return false;
    }

    uint32_t bar0_raw = pci_config_read32(blk_dev.bus, blk_dev.slot, blk_dev.func, 0x10);
    if (!(bar0_raw & 0x1)) {
        return false; // erwartete I/O-Space-BAR, nicht Memory-Space
    }
    uint16_t io_base = (uint16_t)(bar0_raw & ~0x3);
    dev->io_base = io_base;

    // Bus Mastering aktivieren
    uint32_t cmd_status = pci_config_read32(blk_dev.bus, blk_dev.slot, blk_dev.func, 0x04);
    cmd_status |= (1 << 2);
    pci_config_write32(blk_dev.bus, blk_dev.slot, blk_dev.func, 0x04, cmd_status);

    // Status-Handshake
    outb(io_base + VIRTIO_STATUS_OFFSET, 0x00); // RESET

    uint8_t status = inb(io_base + VIRTIO_STATUS_OFFSET);
    status |= 0x01; // ACKNOWLEDGE
    outb(io_base + VIRTIO_STATUS_OFFSET, status);

    status = inb(io_base + VIRTIO_STATUS_OFFSET);
    status |= 0x02; // DRIVER
    outb(io_base + VIRTIO_STATUS_OFFSET, status);

    // Feature-Negotiation: keine optionalen Features
    outl(io_base + 0x04, 0x00000000);

    // Queue 0 auswählen, Größe abfragen
    outw(io_base + 0x0E, 0);
    uint16_t queue_size = inw(io_base + 0x0C);
    dev->queue_size = queue_size;

    // Speicherlayout für Descriptor Table + Available Ring + Used Ring berechnen
    uint32_t desc_table_size = 16 * queue_size;
    uint32_t avail_ring_size = 6 + 2 * queue_size;
    uint32_t used_ring_offset = align_up(desc_table_size + avail_ring_size, 4096);
    uint32_t used_ring_size = 6 + 8 * queue_size;
    uint32_t total_size = used_ring_offset + used_ring_size;
    uint32_t total_pages = align_up(total_size, 4096) / 4096;

    void* queue_mem_phys = pmm->alloc_frames(total_pages);
    if (queue_mem_phys == nullptr) {
        return false;
    }

    uint32_t queue_pfn = (uint32_t)((uint64_t)queue_mem_phys / 4096);
    outl(io_base + 0x08, queue_pfn);

    uint8_t* queue_mem_virt = (uint8_t*)((uint64_t)queue_mem_phys + pmm->get_hhdm());
    dev->desc  = (virtq_desc*)(queue_mem_virt);
    dev->avail = (virtq_avail*)(queue_mem_virt + desc_table_size);
    dev->used  = (virtq_used*)(queue_mem_virt + used_ring_offset);
    dev->next_avail_idx = 0;

    // DRIVER_OK
    status = inb(io_base + VIRTIO_STATUS_OFFSET);
    status |= 0x04;
    outb(io_base + VIRTIO_STATUS_OFFSET, status);

    return true;
}

// Gemeinsame Request-Logik für Read und Write
static bool virtio_blk_request(VirtioBlkDevice* dev, PMM* pmm, uint64_t sector, void* buf, bool is_write) {
    void* req_mem_phys = pmm->alloc_frame();
    if (req_mem_phys == nullptr) return false;
    uint8_t* req_mem_virt = (uint8_t*)((uint64_t)req_mem_phys + pmm->get_hhdm());

    virtio_blk_req* req = (virtio_blk_req*)req_mem_virt;
    uint8_t* data = req_mem_virt + 512;
    volatile uint8_t* status_byte = req_mem_virt + 1024;

    req->type = is_write ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN;
    req->reserved = 0;
    req->sector = sector;

    if (is_write) {
        for (int i = 0; i < 512; i++) data[i] = ((const uint8_t*)buf)[i];
    }

    virtq_desc* desc = dev->desc;

    desc[0].addr  = (uint64_t)req_mem_phys;
    desc[0].len   = sizeof(virtio_blk_req);
    desc[0].flags = VIRTQ_DESC_F_NEXT;
    desc[0].next  = 1;

    desc[1].addr  = (uint64_t)req_mem_phys + 512;
    desc[1].len   = 512;
    desc[1].flags = VIRTQ_DESC_F_NEXT | (is_write ? 0 : VIRTQ_DESC_F_WRITE);
    desc[1].next  = 2;

    desc[2].addr  = (uint64_t)req_mem_phys + 1024;
    desc[2].len   = 1;
    desc[2].flags = VIRTQ_DESC_F_WRITE;
    desc[2].next  = 0;

    dev->avail->ring[dev->avail->idx % dev->queue_size] = 0;
    dev->avail->idx++;

    outw(dev->io_base + 0x10, 0);

    volatile uint16_t* used_idx = &dev->used->idx;
    uint32_t timeout = 10000000;
    uint16_t target = dev->avail->idx; // wie viele Requests insgesamt fertig sein sollen
    while (*used_idx < target && timeout--) {
        asm volatile("pause");
    }
    if (*used_idx < target) {
        return false; // Timeout
    }

    if (*status_byte != 0) {
        return false; // Gerät meldet Fehler
    }

    if (!is_write) {
        for (int i = 0; i < 512; i++) ((uint8_t*)buf)[i] = data[i];
    }

    return true;
}

bool virtio_blk_read(VirtioBlkDevice* dev, PMM* pmm, uint64_t sector, void* buf) {
    return virtio_blk_request(dev, pmm, sector, buf, false);
}

bool virtio_blk_write(VirtioBlkDevice* dev, PMM* pmm, uint64_t sector, const void* buf) {
    return virtio_blk_request(dev, pmm, sector, (void*)buf, true);
}
