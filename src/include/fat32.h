#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <time.h>

// FAT32 constants
#define SECTOR_SIZE 512
#define CLUSTER_SIZE 4096
#define MAX_FILENAME 256
#define FAT_ENTRY_SIZE 32

// File attributes
#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE   0x20

// FAT32 entry structure
typedef struct {
    char filename[MAX_FILENAME];
    uint32_t fileSize;
    uint32_t startCluster;
    uint8_t attributes;
    time_t creationTime;
    time_t modificationTime;
    uint32_t bitmapAddress;
} FAT32_Entry;

// FAT32 file system structure
typedef struct {
    uint32_t totalSectors;
    uint32_t sectorsPerCluster;
    uint32_t reservedSectors;
    uint32_t numberOfFATs;
    uint32_t sectorsPerFAT;
    uint32_t rootCluster;
    uint8_t* fatTable;
    uint8_t* data;
    uint32_t dataSize;
    uint8_t* bitmap;
    uint32_t bitmapSize;
} FAT32_FileSystem;

// Core function declarations
FAT32_FileSystem* fat32_init(uint32_t size);
FAT32_Entry* create_file_entry(FAT32_FileSystem* fs, const char* filename, uint32_t size);
int fat32_write(FAT32_FileSystem* fs, FAT32_Entry* entry, const void* data, uint32_t size);
void* fat32_read(FAT32_FileSystem* fs, FAT32_Entry* entry);
int fat32_delete(FAT32_FileSystem* fs, FAT32_Entry* entry);
void fat32_cleanup(FAT32_FileSystem* fs);

// Helper function declarations
uint32_t allocate_clusters(FAT32_FileSystem* fs, uint32_t count);
void free_clusters(FAT32_FileSystem* fs, uint32_t startCluster);
uint32_t get_next_cluster(FAT32_FileSystem* fs, uint32_t cluster);
void set_next_cluster(FAT32_FileSystem* fs, uint32_t cluster, uint32_t next);
uint32_t cluster_to_sector(FAT32_FileSystem* fs, uint32_t cluster);

#endif
