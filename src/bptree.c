#include "include/bptree.h"
#include "include/distributed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Find position in node
int findPosition(BPTreeNode* node, const char* key) {
    int i;
    for (i = 0; i < node->numKeys; i++) {
        if (strcmp(key, node->keys[i]) < 0) break;
    }
    return i;
}

// Bitmap management
uint32_t allocateBitmapSpace(BPTree* tree) {
    for (uint32_t i = 0; i < tree->bitmapSize * 8; i++) {
        if (!(tree->bitmap[i / 8] & (1 << (i % 8)))) {
            tree->bitmap[i / 8] |= (1 << (i % 8));
            return i;
        }
    }
    return 0xFFFFFFFF;
}

void freeBitmapSpace(BPTree* tree, uint32_t address) {
    if (address < tree->bitmapSize * 8) {
        tree->bitmap[address / 8] &= ~(1 << (address % 8));
    }
}

// Find leaf node containing key
BPTreeNode* findLeaf(BPTreeNode* root, const char* key) {
    BPTreeNode* current = root;
    while (!current->isLeaf) {
        int pos = findPosition(current, key);
        current = current->children[pos];
    }
    return current;
}

// Split leaf node
void splitLeaf(BPTreeNode* parent, int index, BPTreeNode* child) {
    BPTreeNode* newNode = createNode(true);
    int mid = (MAX_KEYS + 1) / 2;

    for (int i = mid; i < child->numKeys; i++) {
        strcpy(newNode->keys[i - mid], child->keys[i]);
        newNode->values[i - mid] = child->values[i];
        newNode->numKeys++;
    }
    child->numKeys = mid;

    newNode->next = child->next;
    child->next = newNode;

    for (int i = parent->numKeys; i > index; i--) {
        strcpy(parent->keys[i], parent->keys[i - 1]);
        parent->children[i + 1] = parent->children[i];
    }
    strcpy(parent->keys[index], newNode->keys[0]);
    parent->children[index + 1] = newNode;
    parent->numKeys++;
}

// Insert into non-full node
void insertNonFull(BPTreeNode* node, const char* key, FAT32_Entry* value) {
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

// Balancing operations
void borrowFromLeft(BPTreeNode* node, BPTreeNode* leftSibling, BPTreeNode* parent, int index) {
    for (int i = node->numKeys; i > 0; i--) {
        strcpy(node->keys[i], node->keys[i-1]);
        node->values[i] = node->values[i-1];
    }
    
    strcpy(node->keys[0], leftSibling->keys[leftSibling->numKeys-1]);
    node->values[0] = leftSibling->values[leftSibling->numKeys-1];
    strcpy(parent->keys[index-1], node->keys[0]);
    
    leftSibling->numKeys--;
    node->numKeys++;
}

void borrowFromRight(BPTreeNode* node, BPTreeNode* rightSibling, BPTreeNode* parent, int index) {
    strcpy(node->keys[node->numKeys], rightSibling->keys[0]);
    node->values[node->numKeys] = rightSibling->values[0];
    
    for (int i = 0; i < rightSibling->numKeys - 1; i++) {
        strcpy(rightSibling->keys[i], rightSibling->keys[i+1]);
        rightSibling->values[i] = rightSibling->values[i+1];
    }
    
    strcpy(parent->keys[index], rightSibling->keys[0]);
    node->numKeys++;
    rightSibling->numKeys--;
}

void mergeNodes(BPTreeNode* leftNode, BPTreeNode* rightNode) {
    int startIndex = leftNode->numKeys;
    
    for (int i = 0; i < rightNode->numKeys; i++) {
        strcpy(leftNode->keys[startIndex + i], rightNode->keys[i]);
        leftNode->values[startIndex + i] = rightNode->values[i];
    }
    
    leftNode->next = rightNode->next;
    leftNode->numKeys += rightNode->numKeys;
    
    free(rightNode);
}

// Core function implementations
BPTree* initializeBPTree(FAT32_FileSystem* fs) {
    BPTree* tree = (BPTree*)malloc(sizeof(BPTree));
    tree->root = createNode(true);
    tree->bitmap = (uint8_t*)calloc(BITMAP_SIZE, sizeof(uint8_t));
    tree->bitmapSize = BITMAP_SIZE;
    tree->fs = fs;
    pthread_rwlock_init(&tree->lock, NULL);
    return tree;
}

void insert(BPTree* tree, const char* key, FAT32_Entry* value) {
    
    pthread_rwlock_wrlock(&tree->lock);
    
    if (tree->root->numKeys == MAX_KEYS) {
        BPTreeNode* newRoot = createNode(false);
        newRoot->children[0] = tree->root;
        tree->root = newRoot;
        splitLeaf(newRoot, 0, newRoot->children[0]);
        insertNonFull(newRoot, key, value);
    } else {
        insertNonFull(tree->root, key, value);
    }
    
    pthread_rwlock_unlock(&tree->lock);
}

void insert_dme(BPTree* tree, const char* key, FAT32_Entry* value, DistributedNode *node) {
    requestToken(node);
    pthread_rwlock_wrlock(&tree->lock);
    
    if (tree->root->numKeys == MAX_KEYS) {
        BPTreeNode* newRoot = createNode(false);
        newRoot->children[0] = tree->root;
        tree->root = newRoot;
        splitLeaf(newRoot, 0, newRoot->children[0]);
        insertNonFull(newRoot, key, value);
    } else {
        insertNonFull(tree->root, key, value);
    }
    
    pthread_rwlock_unlock(&tree->lock);
    releaseToken(node);
}

FAT32_Entry* search(BPTree* tree, const char* key) {
    pthread_rwlock_rdlock(&tree->lock);
    
    BPTreeNode* leaf = findLeaf(tree->root, key);
    for (int i = 0; i < leaf->numKeys; i++) {
        if (strcmp(leaf->keys[i], key) == 0) {
            FAT32_Entry* value = leaf->values[i];
            pthread_rwlock_unlock(&tree->lock);
            return value;
        }
    }
    
    pthread_rwlock_unlock(&tree->lock);
    return NULL;
}

void delete(BPTree* tree, const char* key) {
    pthread_rwlock_wrlock(&tree->lock);
    
    BPTreeNode* leaf = findLeaf(tree->root, key);
    int i;
    for (i = 0; i < leaf->numKeys; i++) {
        if (strcmp(leaf->keys[i], key) == 0) {
            freeBitmapSpace(tree, leaf->bitmapAddress);
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

void delete_dme(BPTree* tree, const char* key, DistributedNode *node) {
    requestToken(node);
    pthread_rwlock_wrlock(&tree->lock);
    
    BPTreeNode* leaf = findLeaf(tree->root, key);
    int i;
    for (i = 0; i < leaf->numKeys; i++) {
        if (strcmp(leaf->keys[i], key) == 0) {
            freeBitmapSpace(tree, leaf->bitmapAddress);
            for (int j = i; j < leaf->numKeys - 1; j++) {
                strcpy(leaf->keys[j], leaf->keys[j + 1]);
                leaf->values[j] = leaf->values[j + 1];
            }
            leaf->numKeys--;
            break;
        }
    }
    
    pthread_rwlock_unlock(&tree->lock);
    releaseToken(node);
}

bool update(BPTree* tree, const char* oldKey, const char* newKey, FAT32_Entry* newValue) {
    pthread_rwlock_wrlock(&tree->lock);
    
    BPTreeNode* leaf = findLeaf(tree->root, oldKey);
    bool found = false;
    
    for (int i = 0; i < leaf->numKeys; i++) {
        if (strcmp(leaf->keys[i], oldKey) == 0) {
            if ((i == 0 || strcmp(leaf->keys[i-1], newKey) < 0) &&
                (i == leaf->numKeys-1 || strcmp(leaf->keys[i+1], newKey) > 0)) {
                strcpy(leaf->keys[i], newKey);
                leaf->values[i] = newValue;
                found = true;
            } else {
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

bool update_dme(BPTree* tree, const char* oldKey, const char* newKey, FAT32_Entry* newValue, DistributedNode *node) {
    requestToken(node);
    pthread_rwlock_wrlock(&tree->lock);
    
    BPTreeNode* leaf = findLeaf(tree->root, oldKey);
    bool found = false;
    
    for (int i = 0; i < leaf->numKeys; i++) {
        if (strcmp(leaf->keys[i], oldKey) == 0) {
            if ((i == 0 || strcmp(leaf->keys[i-1], newKey) < 0) &&
                (i == leaf->numKeys-1 || strcmp(leaf->keys[i+1], newKey) > 0)) {
                strcpy(leaf->keys[i], newKey);
                leaf->values[i] = newValue;
                found = true;
            } else {
                delete(tree, oldKey);
                insert(tree, newKey, newValue);
                found = true;
            }
            break;
        }
    }
    
    pthread_rwlock_unlock(&tree->lock);
    releaseToken(node);
    return found;
}

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
