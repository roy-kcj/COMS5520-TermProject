#ifndef FAT32_H
#define FAT32_H

typedef struct FATEntry {
    int cluster_number;
    char filename[256];
} FATEntry;

void fat32_insert(const char *filename, const char *data);
void fat32_delete(const char *filename);
char* fat32_search(const char *filename);

#endif
