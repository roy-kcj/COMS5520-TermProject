#ifndef BPTREE_H
#define BPTREE_H

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include "fat32.h"

// Constants for B+ Tree configuration
#define MAX_KEYS 4
#define MIN_KEYS (MAX_KEYS/2)
#define MAX_FILENAME 256
#define BITMAP_SIZE 1024

// Node structure for B+ Tree
typedef struct BPTreeNode {
    bool isLeaf;                         // Flag indicating if node is a leaf
    int numKeys;                         // Current number of keys in node
    char keys[MAX_KEYS][MAX_FILENAME];   // Array of keys (filenames)
    FAT32_Entry* values[MAX_KEYS];       // Array of FAT32 entries
    struct BPTreeNode* children[MAX_KEYS + 1];  // Pointers to child nodes
    struct BPTreeNode* next;             // Pointer to next leaf (for leaf nodes)
    uint32_t bitmapAddress;              // Bitmap location for directory entries
} BPTreeNode;

// B+ Tree structure
typedef struct {
    BPTreeNode* root;                    // Pointer to root node
    pthread_rwlock_t lock;               // Read-write lock for thread safety
    uint8_t* bitmap;                     // Bitmap for directory management
    uint32_t bitmapSize;                 // Size of bitmap in bytes
    FAT32_FileSystem* fs;                // Pointer to FAT32 file system
} BPTree;

// Core function declarations
BPTree* initializeBPTree(FAT32_FileSystem* fs);
void insert(BPTree* tree, const char* key, FAT32_Entry* value);
FAT32_Entry* search(BPTree* tree, const char* key);
void delete(BPTree* tree, const char* key);
bool update(BPTree* tree, const char* oldKey, const char* newKey, FAT32_Entry* newValue);
void destroyBPTree(BPTree* tree);

#endif
