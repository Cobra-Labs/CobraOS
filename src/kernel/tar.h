#pragma once
#include <stdint.h>
#include <stddef.h>

struct TARHeader {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char type;
    char linkname[100];
    char magic[6];
    char padding[255];
} __attribute__((packed));

// Gibt Pointer auf Dateiinhalt zurück, size wird gesetzt
const uint8_t* tar_find(const uint8_t* tar, uint64_t tar_size, 
                         const char* filename, uint64_t* size);
