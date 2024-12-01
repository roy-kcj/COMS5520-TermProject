// fat32.c
#include "fat32.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static BPTreeNode* create_node(bool is_leaf) {
    BPTreeNode* node = (BPTreeNode*)malloc(sizeof(BPTreeNode));
    node->is_leaf = is_leaf;
    node->key_count = 0;
    node->next = NULL;
    memset(node->keys, 0, sizeof(node->keys));
    memset(node->children, 0, sizeof(node->children));
    memset(node->entries, 0, sizeof(node->entries));
    return node;
}

static uint32_t allocate_cluster(FAT32_FileSystem* fs) {
    for (uint32_t i = 0; i < fs->total_clusters; i++) {
        if (!(fs->bitmap[i / 8] & (1 << (i % 8)))) {
            fs->bitmap[i / 8] |= (1 << (i % 8));
            return i;
        }
    }
    return 0xFFFFFFFF;
}

FAT32_FileSystem* fat32_init(uint32_t size) {
    FAT32_FileSystem* fs = (FAT32_FileSystem*)malloc(sizeof(FAT32_FileSystem));
    fs->total_clusters = size / CLUSTER_SIZE;
    fs->fat_table = (uint32_t*)calloc(fs->total_clusters, sizeof(uint32_t));
    fs->bitmap = (uint8_t*)calloc(fs->total_clusters / 8 + 1, sizeof(uint8_t));
    fs->root = create_node(true);
    return fs;
}

static void split_child(BPTreeNode* parent, int index, BPTreeNode* child) {
    BPTreeNode* new_node = create_node(child->is_leaf);
    int mid = (B_TREE_ORDER - 1) / 2;

    for (int i = 0; i < B_TREE_ORDER - mid - 1; i++) {
        strcpy(new_node->keys[i], child->keys[i + mid + 1]);
        new_node->entries[i] = child->entries[i + mid + 1];
        child->entries[i + mid + 1] = NULL;
    }

    if (!child->is_leaf) {
        for (int i = 0; i < B_TREE_ORDER - mid; i++) {
            new_node->children[i] = child->children[i + mid + 1];
            child->children[i + mid + 1] = NULL;
        }
    }

    new_node->key_count = B_TREE_ORDER - mid - 1;
    child->key_count = mid;

    for (int i = parent->key_count; i > index; i--) {
        strcpy(parent->keys[i], parent->keys[i - 1]);
        parent->children[i + 1] = parent->children[i];
    }

    strcpy(parent->keys[index], child->keys[mid]);
    parent->children[index + 1] = new_node;
    parent->key_count++;

    if (child->is_leaf) {
        new_node->next = child->next;
        child->next = new_node;
    }
}

