// fat32.h
#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define CLUSTER_SIZE 4096
#define MAX_FILENAME 256
#define B_TREE_ORDER 4

typedef struct {
    char filename[MAX_FILENAME];
    uint32_t start_cluster;
    uint32_t file_size;
    uint8_t attributes;
    time_t created_time;
    time_t modified_time;
} FAT32_Entry;

typedef struct BPTreeNode {
    bool is_leaf;
    int key_count;
    char keys[B_TREE_ORDER][MAX_FILENAME];
    struct BPTreeNode* children[B_TREE_ORDER + 1];
    FAT32_Entry* entries[B_TREE_ORDER];
    struct BPTreeNode* next;
} BPTreeNode;

typedef struct {
    uint32_t total_clusters;
    uint32_t* fat_table;
    uint8_t* bitmap;
    BPTreeNode* root;
} FAT32_FileSystem;

// Function declarations
FAT32_FileSystem* fat32_init(uint32_t size);
void fat32_insert(FAT32_FileSystem* fs, const char* filename, const char* data);
void fat32_delete(FAT32_FileSystem* fs, const char* filename);
char* fat32_search(FAT32_FileSystem* fs, const char* filename);
void fat32_cleanup(FAT32_FileSystem* fs);

#endif
