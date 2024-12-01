#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX_KEYS 4  // B+ Tree order based on paper specifications
#define MAX_FILENAME 256
#define BITMAP_SIZE 1024  // For directory entry management

// Node structure for B+ Tree
typedef struct BPTreeNode {
    bool isLeaf;
    int numKeys;
    char keys[MAX_KEYS][MAX_FILENAME];
    void* values[MAX_KEYS];  // Generic pointer for values
    struct BPTreeNode* children[MAX_KEYS + 1];
    struct BPTreeNode* next;  // For leaf node linking
    uint32_t bitmapAddress;   // Bitmap location for directory entries
} BPTreeNode;

// B+ Tree structure
typedef struct {
    BPTreeNode* root;
    pthread_rwlock_t lock;
    uint8_t* bitmap;         // Bitmap for directory management
    uint32_t bitmapSize;
} BPTree;

// Create new node
BPTreeNode* createNode(bool isLeaf) {
    BPTreeNode* node = (BPTreeNode*)malloc(sizeof(BPTreeNode));
    node->isLeaf = isLeaf;
    node->numKeys = 0;
    node->next = NULL;
    node->bitmapAddress = 0;
    memset(node->keys, 0, sizeof(node->keys));
    memset(node->values, 0, sizeof(node->values));
    memset(node->children, 0, sizeof(node->children));
    return node;
}

// Initialize B+ Tree
BPTree* initializeBPTree() {
    BPTree* tree = (BPTree*)malloc(sizeof(BPTree));
    tree->root = createNode(true);
    tree->bitmap = (uint8_t*)calloc(BITMAP_SIZE, sizeof(uint8_t));
    tree->bitmapSize = BITMAP_SIZE;
    pthread_rwlock_init(&tree->lock, NULL);
    return tree;
}

// Find leaf node containing key
BPTreeNode* findLeaf(BPTreeNode* root, const char* key) {
    BPTreeNode* current = root;
    while (!current->isLeaf) {
        int i;
        for (i = 0; i < current->numKeys; i++) {
            if (strcmp(key, current->keys[i]) < 0) break;
        }
        current = current->children[i];
    }
    return current;
}

// Split leaf node
void splitLeaf(BPTreeNode* parent, int index, BPTreeNode* child) {
    BPTreeNode* newNode = createNode(true);
    int mid = (MAX_KEYS + 1) / 2;

    // Copy upper half to new node
    for (int i = mid; i < child->numKeys; i++) {
        strcpy(newNode->keys[i - mid], child->keys[i]);
        newNode->values[i - mid] = child->values[i];
        newNode->numKeys++;
    }
    child->numKeys = mid;

    // Update leaf links
    newNode->next = child->next;
    child->next = newNode;

    // Insert new key and pointer in parent
    for (int i = parent->numKeys; i > index; i--) {
        strcpy(parent->keys[i], parent->keys[i - 1]);
        parent->children[i + 1] = parent->children[i];
    }
    strcpy(parent->keys[index], newNode->keys[0]);
    parent->children[index + 1] = newNode;
    parent->numKeys++;
}

// Insert into non-full node
void insertNonFull(BPTreeNode* node, const char* key, void* value) {
    int i = node->numKeys - 1;
    
    if (node->isLeaf) {
        while (i >= 0 && strcmp(key, node->keys[i]) < 0) {
            strcpy(node->keys[i + 1], node->keys[i]);
            node->values[i + 1] = node->values[i];
            i--;
        }
        strcpy(node->keys[i + 1], key);
        node->values[i + 1] = value;
        node->numKeys++;
    } else {
        while (i >= 0 && strcmp(key, node->keys[i]) < 0) i--;
        i++;
        
        if (node->children[i]->numKeys == MAX_KEYS) {
            splitLeaf(node, i, node->children[i]);
            if (strcmp(key, node->keys[i]) > 0) i++;
        }
        insertNonFull(node->children[i], key, value);
    }
}

// Insert key-value pair
void insert(BPTree* tree, const char* key, void* value) {
    pthread_rwlock_wrlock(&tree->lock);
    
    BPTreeNode* root = tree->root;
    if (root->numKeys == MAX_KEYS) {
        BPTreeNode* newRoot = createNode(false);
        tree->root = newRoot;
        newRoot->children[0] = root;
        splitLeaf(newRoot, 0, root);
        insertNonFull(newRoot, key, value);
    } else {
        insertNonFull(root, key, value);
    }
    
    pthread_rwlock_unlock(&tree->lock);
}

// Delete key from tree
void delete(BPTree* tree, const char* key) {
    pthread_rwlock_wrlock(&tree->lock);
    
    BPTreeNode* leaf = findLeaf(tree->root, key);
    int i;
    for (i = 0; i < leaf->numKeys; i++) {
        if (strcmp(leaf->keys[i], key) == 0) {
            // Shift remaining keys and values
            for (int j = i; j < leaf->numKeys - 1; j++) {
                strcpy(leaf->keys[j], leaf->keys[j + 1]);
                leaf->values[j] = leaf->values[j + 1];
            }
            leaf->numKeys--;
            break;
        }
    }
    
    pthread_rwlock_unlock(&tree->lock);
}

// Update key-value pair
bool update(BPTree* tree, const char* oldKey, const char* newKey, void* newValue) {
    pthread_rwlock_wrlock(&tree->lock);
    
    // Find and delete old key
    BPTreeNode* leaf = findLeaf(tree->root, oldKey);
    bool found = false;
    
    for (int i = 0; i < leaf->numKeys; i++) {
        if (strcmp(leaf->keys[i], oldKey) == 0) {
            // If new key maintains order, update in place
            if ((i == 0 || strcmp(leaf->keys[i-1], newKey) < 0) &&
                (i == leaf->numKeys-1 || strcmp(leaf->keys[i+1], newKey) > 0)) {
                strcpy(leaf->keys[i], newKey);
                leaf->values[i] = newValue;
                found = true;
            } else {
                // Delete and reinsert
                delete(tree, oldKey);
                insert(tree, newKey, newValue);
                found = true;
            }
            break;
        }
    }
    
    pthread_rwlock_unlock(&tree->lock);
    return found;
}

// Clean up tree
void cleanupTree(BPTreeNode* node) {
    if (!node->isLeaf) {
        for (int i = 0; i <= node->numKeys; i++) {
            cleanupTree(node->children[i]);
        }
    }
    free(node);
}

void destroyBPTree(BPTree* tree) {
    cleanupTree(tree->root);
    free(tree->bitmap);
    pthread_rwlock_destroy(&tree->lock);
    free(tree);
}
