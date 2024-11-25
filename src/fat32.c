#include "fat32.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simplified FAT32 insert function
void fat32_insert(const char *filename, const char *data) {
    write_to_file(filename, data);
}

void fat32_delete(const char *filename) {
    remove(filename);
}

char* fat32_search(const char *filename) {
    return read_from_file(filename);
}
