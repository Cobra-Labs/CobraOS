#pragma once
#include <stdint.h>
#include "../drivers/virtio_blk.h"
#include "../paging/pmm.h"

struct Ext2Superblock {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t r_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;
    uint16_t mnt_count;
    uint16_t max_mnt_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;
    uint32_t lastcheck;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t def_resuid;
    uint16_t def_resgid;
    uint32_t first_ino;
    uint16_t inode_size;
    uint16_t block_group_nr;
    uint32_t feature_compat;
    uint32_t feature_incompat;
    uint32_t feature_ro_compat;
    uint8_t  uuid[16];
    char     volume_name[16];
    char     last_mounted[64];
    uint32_t algo_bitmap;
} __attribute__((packed));

#define EXT2_MAGIC 0xEF53

struct Ext2GroupDesc {
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
    uint16_t pad;
    uint8_t  reserved[12];
} __attribute__((packed));

struct Ext2Inode {
    uint16_t mode;
    uint16_t uid;
    uint32_t size_low;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks;
    uint32_t flags;
    uint32_t osd1;
    uint32_t block[15];
    uint32_t generation;
    uint32_t file_acl;
    uint32_t size_high;
    uint32_t faddr;
    uint8_t  osd2[12];
} __attribute__((packed));

#define EXT2_S_IFDIR 0x4000
#define EXT2_S_IFREG 0x8000

struct Ext2DirEntry {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t  name_len;
    uint8_t  file_type;
    char     name[];
} __attribute__((packed));

struct Ext2FS {
    VirtioBlkDevice* blk;
    PMM* pmm;
    Ext2Superblock sb;
    uint32_t block_size;
    uint32_t inode_size;
    uint32_t first_ino;
};

bool ext2_mount(Ext2FS* fs, VirtioBlkDevice* blk, PMM* pmm);
bool ext2_read_block(Ext2FS* fs, uint32_t block_num, void* buf);
bool ext2_read_inode(Ext2FS* fs, uint32_t inode_num, Ext2Inode* out);

// Löst einen absoluten Pfad (z.B. "/foo/bar.txt") zu einer Inode-Nummer auf.
// Gibt 0 zurück, wenn nicht gefunden.
uint32_t ext2_find(Ext2FS* fs, const char* path);

// Liest bis zu max_len Bytes des Dateiinhalts von inode_num in buf.
// Gibt die tatsächlich gelesene Anzahl Bytes zurück, oder 0 bei Fehler.
// Unterstützt aktuell nur direkte Blöcke (Dateien bis 12 * block_size).
uint32_t ext2_read_file(Ext2FS* fs, uint32_t inode_num, void* buf, uint32_t max_len);
