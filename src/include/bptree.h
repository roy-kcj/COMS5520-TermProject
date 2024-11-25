#ifndef BPTREE_H
#define BPTREE_H

#include <pthread.h>

#define MAX_KEYS    25 // n keys and n+1 pointers

typedef struct BPlusTreeNode {
    int is_leaf;
    int num_keys;
    int keys[MAX_KEYS];
    void *values[MAX_KEYS];  // For leaf nodes, store associated data (e.g., file names)
    struct BPlusTreeNode *children[MAX_KEYS + 1];  // For internal nodes
    struct BPlusTreeNode *next;  // Pointer to the next leaf node (used in leaf nodes)
} BPlusTreeNode;

typedef struct BPlusTree {
    BPlusTreeNode *root;
    pthread_rwlock_t lock;
} BPlusTree;

// Function prototypes
BPlusTree* initialize_bplustree();
void bplustree_insert(BPlusTree *tree, int key, const char *value);
void* bplustree_search(BPlusTree *tree, int key);
void bplustree_delete(BPlusTree *tree, int key);
void free_bplustree(BPlusTree *tree);

#endif
