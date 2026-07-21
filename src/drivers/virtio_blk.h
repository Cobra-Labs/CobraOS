#pragma once
#include <stdint.h>
#include "../paging/pmm.h"

#define VIRTIO_STATUS_OFFSET 0x12

struct virtq_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed));

#define VIRTQ_DESC_F_NEXT     1
#define VIRTQ_DESC_F_WRITE    2

struct virtq_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[];
} __attribute__((packed));

struct virtq_used_elem {
    uint32_t id;
    uint32_t len;
} __attribute__((packed));

struct virtq_used {
    uint16_t flags;
    uint16_t idx;
    virtq_used_elem ring[];
} __attribute__((packed));

struct virtio_blk_req {
    uint32_t type;
    uint32_t reserved;
    uint64_t sector;
} __attribute__((packed));

#define VIRTIO_BLK_T_IN  0
#define VIRTIO_BLK_T_OUT 1

struct VirtioBlkDevice {
    uint16_t io_base;
    uint16_t queue_size;
    virtq_desc*  desc;
    virtq_avail* avail;
    virtq_used*  used;
    uint16_t next_avail_idx;
};

bool virtio_blk_init(VirtioBlkDevice* dev, PMM* pmm);
bool virtio_blk_read(VirtioBlkDevice* dev, PMM* pmm, uint64_t sector, void* buf);
bool virtio_blk_write(VirtioBlkDevice* dev, PMM* pmm, uint64_t sector, const void* buf);
