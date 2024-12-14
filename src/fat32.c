#include "include/fat32.h"
#include "include/distributed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Helper function implementations
uint32_t cluster_to_sector(FAT32_FileSystem* fs, uint32_t cluster) {
    return ((cluster - 2) * fs->sectorsPerCluster) + fs->reservedSectors + 
           (fs->numberOfFATs * fs->sectorsPerFAT);
}

uint32_t allocate_clusters(FAT32_FileSystem* fs, uint32_t count) {
    uint32_t start = 0;
    uint32_t current = 0;
    uint32_t found = 0;
    
    for (uint32_t i = 2; i < fs->totalSectors/fs->sectorsPerCluster; i++) {
        if (get_next_cluster(fs, i) == 0) {
            if (found == 0) start = i;
            found++;
            if (found == count) {
                // Link clusters
                for (uint32_t j = start; j < start + count - 1; j++) {
                    set_next_cluster(fs, j, j + 1);
                }
                set_next_cluster(fs, start + count - 1, 0xFFFFFFFF);
                return start;
            }
        } else {
            found = 0;
        }
    }
    return 0;
}

void free_clusters(FAT32_FileSystem* fs, uint32_t startCluster) {
    uint32_t current = startCluster;
    uint32_t next;
    
    while (current != 0xFFFFFFFF && current != 0) {
        next = get_next_cluster(fs, current);
        set_next_cluster(fs, current, 0);
        current = next;
    }
}

// FAT table operations
uint32_t get_next_cluster(FAT32_FileSystem* fs, uint32_t cluster) {
    return fs->fatTable[cluster];
}

void set_next_cluster(FAT32_FileSystem* fs, uint32_t cluster, uint32_t next) {
    fs->fatTable[cluster] = next;
}

// Core function implementations
FAT32_FileSystem* fat32_init(uint32_t size) {
    FAT32_FileSystem* fs = (FAT32_FileSystem*)malloc(sizeof(FAT32_FileSystem));
    
    fs->totalSectors = size / SECTOR_SIZE;
    fs->sectorsPerCluster = CLUSTER_SIZE / SECTOR_SIZE;
    fs->reservedSectors = 32;
    fs->numberOfFATs = 2;
    fs->sectorsPerFAT = (fs->totalSectors / fs->sectorsPerCluster * 4 + SECTOR_SIZE - 1) / SECTOR_SIZE;
    fs->rootCluster = 2;
    
    // Allocate FAT table
    uint32_t fatSize = fs->totalSectors / fs->sectorsPerCluster * sizeof(uint32_t);
    fs->fatTable = (uint8_t*)calloc(fatSize, 1);
    
    // Allocate data region
    fs->dataSize = size - (fs->reservedSectors + fs->numberOfFATs * fs->sectorsPerFAT) * SECTOR_SIZE;
    fs->data = (uint8_t*)calloc(fs->dataSize, 1);
    
    // Initialize bitmap
    fs->bitmapSize = fs->totalSectors / 8 + 1;
    fs->bitmap = (uint8_t*)calloc(fs->bitmapSize, 1);
    
    return fs;
}

FAT32_Entry* create_file_entry(FAT32_FileSystem* fs, const char* filename, uint32_t size) {

    FAT32_Entry* entry = (FAT32_Entry*)malloc(sizeof(FAT32_Entry));
    
    strncpy(entry->filename, filename, MAX_FILENAME - 1);
    entry->fileSize = size;
    entry->attributes = ATTR_ARCHIVE;
    entry->creationTime = entry->modificationTime = time(NULL);
    
    // Allocate clusters
    uint32_t clustersNeeded = (size + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
    entry->startCluster = allocate_clusters(fs, clustersNeeded);
    
    return entry;
}

FAT32_Entry* create_file_entry_dme(FAT32_FileSystem* fs, const char* filename, uint32_t size, DistributedNode *node) {
    requestToken(node);

    FAT32_Entry* entry = (FAT32_Entry*)malloc(sizeof(FAT32_Entry));
    
    strncpy(entry->filename, filename, MAX_FILENAME - 1);
    entry->fileSize = size;
    entry->attributes = ATTR_ARCHIVE;
    entry->creationTime = entry->modificationTime = time(NULL);
    
    // Allocate clusters
    uint32_t clustersNeeded = (size + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
    entry->startCluster = allocate_clusters(fs, clustersNeeded);
    
    releaseToken(node);
    return entry;
}

int fat32_write(FAT32_FileSystem* fs, FAT32_Entry* entry, const void* data, uint32_t size) {


    if (!entry || !data || size == 0) return 0;
    
    uint32_t cluster = entry->startCluster;
    uint32_t remaining = size;
    const uint8_t* buffer = (const uint8_t*)data;
    
    while (remaining > 0 && cluster != 0xFFFFFFFF) {
        uint32_t sector = cluster_to_sector(fs, cluster);
        uint32_t writeSize = (remaining < CLUSTER_SIZE) ? remaining : CLUSTER_SIZE;
        
        memcpy(fs->data + sector * SECTOR_SIZE, buffer, writeSize);
        
        buffer += writeSize;
        remaining -= writeSize;
        cluster = get_next_cluster(fs, cluster);
    }
    
    entry->fileSize = size;
    entry->modificationTime = time(NULL);
    
    return 1;
}

int fat32_write_dme(FAT32_FileSystem* fs, FAT32_Entry* entry, const void* data, uint32_t size, DistributedNode *node) {
    requestToken(node);

    if (!entry || !data || size == 0) return 0;
    
    uint32_t cluster = entry->startCluster;
    uint32_t remaining = size;
    const uint8_t* buffer = (const uint8_t*)data;
    
    while (remaining > 0 && cluster != 0xFFFFFFFF) {
        uint32_t sector = cluster_to_sector(fs, cluster);
        uint32_t writeSize = (remaining < CLUSTER_SIZE) ? remaining : CLUSTER_SIZE;
        
        memcpy(fs->data + sector * SECTOR_SIZE, buffer, writeSize);
        
        buffer += writeSize;
        remaining -= writeSize;
        cluster = get_next_cluster(fs, cluster);
    }
    
    entry->fileSize = size;
    entry->modificationTime = time(NULL);
    
    releaseToken(node);
    return 1;
}

void* fat32_read(FAT32_FileSystem* fs, FAT32_Entry* entry) {
    if (!entry || entry->fileSize == 0) return NULL;
    
    uint8_t* buffer = (uint8_t*)malloc(entry->fileSize);
    uint32_t cluster = entry->startCluster;
    uint32_t remaining = entry->fileSize;
    uint8_t* current = buffer;
    
    while (remaining > 0 && cluster != 0xFFFFFFFF) {
        uint32_t sector = cluster_to_sector(fs, cluster);
        uint32_t readSize = (remaining < CLUSTER_SIZE) ? remaining : CLUSTER_SIZE;
        
        memcpy(current, fs->data + sector * SECTOR_SIZE, readSize);
        
        current += readSize;
        remaining -= readSize;
        cluster = get_next_cluster(fs, cluster);
    }
    
    return buffer;
}

int fat32_delete(FAT32_FileSystem* fs, FAT32_Entry* entry) {
    if (!entry) return 0;
    
    free_clusters(fs, entry->startCluster);
    free(entry);
    
    return 1;
}

void fat32_cleanup(FAT32_FileSystem* fs) {
    free(fs->fatTable);
    free(fs->data);
    free(fs->bitmap);
    free(fs);
}
