#include "tar.h"

// Oktal-String zu Zahl konvertieren
static uint64_t oct_to_int(const char* str) {
    uint64_t result = 0;
    while (*str && *str != ' ') {
        result = result * 8 + (*str - '0');
        str++;
    }
    return result;
}

// Strings vergleichen
static bool str_eq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return false;
        a++; b++;
    }
    return *a == *b;
}

const uint8_t* tar_find(const uint8_t* tar, uint64_t tar_size,
                         const char* filename, uint64_t* size) {
    uint64_t offset = 0;

    while (offset + 512 <= tar_size) {
        TARHeader* header = (TARHeader*)(tar + offset);

        // Leerer Header = Ende
        if (header->name[0] == '\0') break;

        uint64_t file_size = oct_to_int(header->size);

        // Datei gefunden?
        if (header->type == '0' && str_eq(header->name, filename)) {
            *size = file_size;
            return tar + offset + 512;
        }

        // Nächster Eintrag — Header + Daten (auf 512 aufgerundet)
        offset += 512 + ((file_size + 511) / 512) * 512;
    }

    return nullptr;
}
