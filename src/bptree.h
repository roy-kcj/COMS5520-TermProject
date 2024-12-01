#ifndef BPTREE_H
#define BPTREE_H

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

#define MAX_KEYS 4
#define MAX_FILENAME 256
#define BITMAP_SIZE 1024

// Node structure
typedef struct BPTreeNode {
    bool isLeaf;
    int numKeys;
    char keys[MAX_KEYS][MAX_FILENAME];
    void* values[MAX_KEYS];
    struct BPTreeNode* children[MAX_KEYS + 1];
    struct BPTreeNode* next;
    uint32_t bitmapAddress;
} BPTreeNode;

// Tree structure
typedef struct {
    BPTreeNode* root;
    pthread_rwlock_t lock;
    uint8_t* bitmap;
    uint32_t bitmapSize;
} BPTree;

// Core functions
BPTree* initializeBPTree(void);
void insert(BPTree* tree, const char* key, void* value);
void delete(BPTree* tree, const char* key);
bool update(BPTree* tree, const char* oldKey, const char* newKey, void* newValue);
void* search(BPTree* tree, const char* key);
void destroyBPTree(BPTree* tree);

#endif
