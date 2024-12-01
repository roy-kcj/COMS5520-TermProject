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

void fat32_insert(FAT32_FileSystem* fs, const char* filename, const char* data) {
    if (!fs->root->key_count) {
        strcpy(fs->root->keys[0], filename);
        
        FAT32_Entry* entry = (FAT32_Entry*)malloc(sizeof(FAT32_Entry));
        strcpy(entry->filename, filename);
        entry->start_cluster = allocate_cluster(fs);
        entry->file_size = strlen(data);
        entry->created_time = entry->modified_time = time(NULL);
        
        fs->root->entries[0] = entry;
        fs->root->key_count = 1;
        
        // Write data to allocated cluster
        // Implementation of actual disk write would go here
        
        return;
    }

    BPTreeNode* current = fs->root;
    
    if (current->key_count == B_TREE_ORDER - 1) {
        BPTreeNode* new_root = create_node(false);
        fs->root = new_root;
        new_root->children[0] = current;
        split_child(new_root, 0, current);
        
        int i = 0;
        if (strcmp(new_root->keys[0], filename) < 0)
            i++;
        
        BPTreeNode* child = new_root->children[i];
        // Continue insertion in appropriate child
        // Implementation continues...
    }
}

char* fat32_search(FAT32_FileSystem* fs, const char* filename) {
    BPTreeNode* current = fs->root;
    
    while (!current->is_leaf) {
        int i;
        for (i = 0; i < current->key_count; i++) {
            if (strcmp(filename, current->keys[i]) < 0)
                break;
        }
        current = current->children[i];
    }
    
    for (int i = 0; i < current->key_count; i++) {
        if (strcmp(current->keys[i], filename) == 0) {
            // Read data from cluster
            // Implementation of actual disk read would go here
            return strdup("File found"); // Placeholder
        }
    }
    
    return NULL;
}

void fat32_delete(FAT32_FileSystem* fs, const char* filename) {
    BPTreeNode* current = fs->root;
    int found = 0;
    
    // Find and mark the cluster as free in bitmap
    while (!current->is_leaf) {
        int i;
        for (i = 0; i < current->key_count; i++) {
            if (strcmp(filename, current->keys[i]) < 0)
                break;
        }
        current = current->children[i];
    }
    
    for (int i = 0; i < current->key_count; i++) {
        if (strcmp(current->keys[i], filename) == 0) {
            uint32_t cluster = current->entries[i]->start_cluster;
            fs->bitmap[cluster / 8] &= ~(1 << (cluster % 8));
            free(current->entries[i]);
            
            // Shift remaining entries
            for (int j = i; j < current->key_count - 1; j++) {
                strcpy(current->keys[j], current->keys[j + 1]);
                current->entries[j] = current->entries[j + 1];
            }
            current->key_count--;
            found = 1;
            break;
        }
    }
}

void fat32_cleanup(FAT32_FileSystem* fs) {
    // Recursive cleanup of B+ tree nodes
    // Free all allocated memory
    free(fs->fat_table);
    free(fs->bitmap);
    // Implementation of B+ tree cleanup would go here
    free(fs);
}
