#ifndef BPTREE_H
#define BPTREE_H

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

#define MAX_KEYS 4  // Based on paper specifications
#define MAX_FILENAME 256
#define BITMAP_SIZE 1024

// Node structure for B+ Tree
typedef struct BPTreeNode {
    bool isLeaf;
    int numKeys;
    char keys[MAX_KEYS][MAX_FILENAME];
    void* values[MAX_KEYS];  // Generic pointer for values
    struct BPTreeNode* children[MAX_KEYS + 1];
    struct BPTreeNode* next;  // For leaf node linking
    uint32_t bitmapAddress;   // For directory entry management
} BPTreeNode;

// B+ Tree structure
typedef struct {
    BPTreeNode* root;
    pthread_rwlock_t lock;
    uint8_t* bitmap;         // Bitmap for directory management
    uint32_t bitmapSize;
} BPTree;

// Core function declarations
BPTree* initializeBPTree(void);
void insert(BPTree* tree, const char* key, void* value);
void* search(BPTree* tree, const char* key);
void delete(BPTree* tree, const char* key);
bool update(BPTree* tree, const char* oldKey, const char* newKey, void* newValue);
void destroyBPTree(BPTree* tree);

#endif
