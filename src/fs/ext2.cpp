#include "ext2.h"

bool ext2_mount(Ext2FS* fs, VirtioBlkDevice* blk, PMM* pmm) {
    fs->blk = blk;
    fs->pmm = pmm;

    uint8_t superblock_buf[1024];
    if (!virtio_blk_read(blk, pmm, 2, superblock_buf)) return false;
    if (!virtio_blk_read(blk, pmm, 3, superblock_buf + 512)) return false;

    Ext2Superblock* sb = (Ext2Superblock*)superblock_buf;
    if (sb->magic != EXT2_MAGIC) return false;

    fs->sb = *sb;
    fs->block_size = 1024 << sb->log_block_size;
    fs->inode_size = (sb->rev_level == 0) ? 128 : sb->inode_size;
    fs->first_ino  = (sb->rev_level == 0) ? 11  : sb->first_ino;

    return true;
}

bool ext2_read_block(Ext2FS* fs, uint32_t block_num, void* buf) {
    uint32_t sectors_per_block = fs->block_size / 512;
    uint32_t start_sector = block_num * sectors_per_block;

    uint8_t* out = (uint8_t*)buf;
    for (uint32_t i = 0; i < sectors_per_block; i++) {
        if (!virtio_blk_read(fs->blk, fs->pmm, start_sector + i, out + i * 512)) {
            return false;
        }
    }
    return true;
}

// TODO: unterstützt aktuell nur Gruppe 0 -- reicht für kleine Test-Images,
// muss erweitert werden sobald Inodes/Blöcke aus höheren Gruppen gebraucht werden.
static bool ext2_read_group_desc(Ext2FS* fs, uint32_t group_num, Ext2GroupDesc* out) {
    uint8_t bgdt_buf[1024];
    uint32_t bgdt_block = (fs->block_size == 1024) ? 2 : 1;
    if (!ext2_read_block(fs, bgdt_block, bgdt_buf)) return false;

    Ext2GroupDesc* table = (Ext2GroupDesc*)bgdt_buf;
    *out = table[group_num];
    return true;
}

bool ext2_read_inode(Ext2FS* fs, uint32_t inode_num, Ext2Inode* out) {
    uint32_t inode_index = inode_num - 1;
    uint32_t group_num = inode_index / fs->sb.inodes_per_group;
    uint32_t index_in_group = inode_index % fs->sb.inodes_per_group;

    Ext2GroupDesc gd;
    if (!ext2_read_group_desc(fs, group_num, &gd)) return false;

    uint32_t inodes_per_block = fs->block_size / fs->inode_size;
    uint32_t block_offset = index_in_group / inodes_per_block;
    uint32_t offset_in_block = (index_in_group % inodes_per_block) * fs->inode_size;

    uint8_t block_buf[4096]; // reicht bis block_size=4096
    if (!ext2_read_block(fs, gd.inode_table + block_offset, block_buf)) return false;

    *out = *(Ext2Inode*)(block_buf + offset_in_block);
    return true;
}

// Sucht 'name' (name_len Bytes, nicht null-terminiert) im Verzeichnis-Inode dir_inode.
// Gibt die gefundene Inode-Nummer zurück, oder 0 wenn nicht gefunden.
static uint32_t ext2_find_in_dir(Ext2FS* fs, Ext2Inode* dir_inode, const char* name, uint32_t name_len) {
    uint8_t block_buf[4096];

    // TODO: unterstützt aktuell nur direkte Blöcke, keine indirekten --
    // reicht für Verzeichnisse bis 12 * block_size an Einträgen.
    for (int b = 0; b < 12; b++) {
        if (dir_inode->block[b] == 0) break;

        if (!ext2_read_block(fs, dir_inode->block[b], block_buf)) return 0;

        uint32_t offset = 0;
        while (offset < fs->block_size) {
            Ext2DirEntry* entry = (Ext2DirEntry*)(block_buf + offset);

            if (entry->inode != 0 && entry->name_len == name_len) {
                bool match = true;
                for (uint32_t i = 0; i < name_len; i++) {
                    if (entry->name[i] != name[i]) { match = false; break; }
                }
                if (match) return entry->inode;
            }

            if (entry->rec_len == 0) break;
            offset += entry->rec_len;
        }
    }
    return 0;
}

uint32_t ext2_find(Ext2FS* fs, const char* path) {
    if (path[0] != '/') return 0;

    uint32_t current_inode_num = 2; // Root
    uint32_t i = 1; // '/' überspringen

    while (path[i] != '\0') {
        // aktuelles Pfadsegment ausmessen
        uint32_t seg_start = i;
        while (path[i] != '/' && path[i] != '\0') i++;
        uint32_t seg_len = i - seg_start;

        if (seg_len == 0) {
            // doppeltes '/', überspringen
            if (path[i] == '/') i++;
            continue;
        }

        Ext2Inode dir_inode;
        if (!ext2_read_inode(fs, current_inode_num, &dir_inode)) return 0;
        if (!(dir_inode.mode & EXT2_S_IFDIR)) return 0; // kein Verzeichnis

        uint32_t next_inode_num = ext2_find_in_dir(fs, &dir_inode, path + seg_start, seg_len);
        if (next_inode_num == 0) return 0; // nicht gefunden

        current_inode_num = next_inode_num;

        if (path[i] == '/') i++;
    }

    return current_inode_num;
}

uint32_t ext2_read_file(Ext2FS* fs, uint32_t inode_num, void* buf, uint32_t max_len) {
    Ext2Inode inode;
    if (!ext2_read_inode(fs, inode_num, &inode)) return 0;
    if (!(inode.mode & EXT2_S_IFREG)) return 0;

    uint32_t total = inode.size_low;
    if (total > max_len) total = max_len;

    uint8_t* out = (uint8_t*)buf;
    uint32_t remaining = total;
    uint32_t written = 0;

    // direkte Blöcke (0-11)
    for (int b = 0; b < 12 && remaining > 0; b++) {
        if (inode.block[b] == 0) break;

        uint8_t block_buf[4096];
        if (!ext2_read_block(fs, inode.block[b], block_buf)) break;

        uint32_t chunk = (remaining < fs->block_size) ? remaining : fs->block_size;
        for (uint32_t i = 0; i < chunk; i++) out[written + i] = block_buf[i];

        written += chunk;
        remaining -= chunk;
    }

    // einfach-indirekter Block (12)
    if (remaining > 0 && inode.block[12] != 0) {
        uint32_t pointers_per_block = fs->block_size / 4;
        uint8_t indirect_buf[4096];
        if (ext2_read_block(fs, inode.block[12], indirect_buf)) {
            uint32_t* pointers = (uint32_t*)indirect_buf;

            for (uint32_t p = 0; p < pointers_per_block && remaining > 0; p++) {
                if (pointers[p] == 0) break;

                uint8_t block_buf[4096];
                if (!ext2_read_block(fs, pointers[p], block_buf)) break;

                uint32_t chunk = (remaining < fs->block_size) ? remaining : fs->block_size;
                for (uint32_t i = 0; i < chunk; i++) out[written + i] = block_buf[i];

                written += chunk;
                remaining -= chunk;
            }
        }
    }

    // TODO: doppelt/dreifach-indirekt fuer noch groessere Dateien

    return written;
}
