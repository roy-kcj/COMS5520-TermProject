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
