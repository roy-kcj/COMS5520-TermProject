#include "include/bptree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Helper function: Create a new B+ Tree node
BPlusTreeNode* create_node(int is_leaf) {
    BPlusTreeNode *node = (BPlusTreeNode *)malloc(sizeof(BPlusTreeNode));
    node->is_leaf = is_leaf;
    node->num_keys = 0;
    memset(node->keys, 0, sizeof(node->keys));
    memset(node->values, 0, sizeof(node->values));
    memset(node->children, 0, sizeof(node->children));
    node->next = NULL;
    return node;
}

// Initialize a new B+ Tree
BPlusTree* initialize_bplustree() {
    BPlusTree *tree = (BPlusTree *)malloc(sizeof(BPlusTree));
    tree->root = create_node(1);  // Root is initially a leaf node
    pthread_rwlock_init(&tree->lock, NULL);
    return tree;
}

// Helper function: Find the leaf node where a key should reside
BPlusTreeNode* find_leaf(BPlusTreeNode *root, int key) {
    BPlusTreeNode *node = root;
    while (!node->is_leaf) {
        int i = 0;
        while (i < node->num_keys && key >= node->keys[i]) {
            i++;
        }
        node = node->children[i];
    }
    return node;
}

// Insert a key-value pair into the B+ Tree
void bplustree_insert(BPlusTree *tree, int key, const char *value) {
    pthread_rwlock_wrlock(&tree->lock);  // Lock for writing

    BPlusTreeNode *root = tree->root;
    BPlusTreeNode *leaf = find_leaf(root, key);

    // Insert into the leaf node
    int i;
    for (i = leaf->num_keys - 1; i >= 0 && key < leaf->keys[i]; i--) {
        leaf->keys[i + 1] = leaf->keys[i];
        leaf->values[i + 1] = leaf->values[i];
    }
    leaf->keys[i + 1] = key;
    leaf->values[i + 1] = strdup(value);
    leaf->num_keys++;

    // If the leaf is full, split it
    if (leaf->num_keys == MAX_KEYS) {
        BPlusTreeNode *new_leaf = create_node(1);
        int mid = MAX_KEYS / 2;

        // Move the second half of keys and values to the new leaf
        new_leaf->num_keys = MAX_KEYS - mid;
        for (i = 0; i < new_leaf->num_keys; i++) {
            new_leaf->keys[i] = leaf->keys[mid + i];
            new_leaf->values[i] = leaf->values[mid + i];
        }
        leaf->num_keys = mid;

        // Link the new leaf
        new_leaf->next = leaf->next;
        leaf->next = new_leaf;

        // Promote the new key to the parent
        int promote_key = new_leaf->keys[0];
        if (leaf == root) {
            // Create a new root if necessary
            BPlusTreeNode *new_root = create_node(0);
            new_root->keys[0] = promote_key;
            new_root->children[0] = leaf;
            new_root->children[1] = new_leaf;
            new_root->num_keys = 1;
            tree->root = new_root;
        } else {
            // Insert into parent
            BPlusTreeNode *parent = root;
            BPlusTreeNode *current = parent;
            while (!current->is_leaf) {
                parent = current;
                current = current->children[current->num_keys - 1];
            }
            for (i = parent->num_keys - 1; i >= 0 && promote_key < parent->keys[i]; i--) {
                parent->keys[i + 1] = parent->keys[i];
                parent->children[i + 2] = parent->children[i + 1];
            }
            parent->keys[i + 1] = promote_key;
            parent->children[i + 2] = new_leaf;
            parent->num_keys++;
        }
    }

    pthread_rwlock_unlock(&tree->lock);
}

// Search for a key in the B+ Tree
void* bplustree_search(BPlusTree *tree, int key) {
    pthread_rwlock_rdlock(&tree->lock);  // Lock for reading

    BPlusTreeNode *leaf = find_leaf(tree->root, key);
    for (int i = 0; i < leaf->num_keys; i++) {
        if (leaf->keys[i] == key) {
            pthread_rwlock_unlock(&tree->lock);
            return leaf->values[i];  // Return the associated value
        }
    }

    pthread_rwlock_unlock(&tree->lock);
    return NULL;  // Key not found
}

// Delete a key from the B+ Tree
void bplustree_delete(BPlusTree *tree, int key) {
    pthread_rwlock_wrlock(&tree->lock);  // Lock for writing

    // Locate the key in the leaf node
    BPlusTreeNode *leaf = find_leaf(tree->root, key);
    int i;
    for (i = 0; i < leaf->num_keys; i++) {
        if (leaf->keys[i] == key) {
            break;
        }
    }
    if (i == leaf->num_keys) {
        pthread_rwlock_unlock(&tree->lock);
        return;  // Key not found
    }

    // Remove the key and shift remaining keys and values
    for (; i < leaf->num_keys - 1; i++) {
        leaf->keys[i] = leaf->keys[i + 1];
        leaf->values[i] = leaf->values[i + 1];
    }
    leaf->num_keys--;

    // Handle underflow (optional for simplicity)

    pthread_rwlock_unlock(&tree->lock);
}

// Free the B+ Tree recursively
void free_bplustree_nodes(BPlusTreeNode *node) {
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            free_bplustree_nodes(node->children[i]);
        }
    }
    free(node);
}

void free_bplustree(BPlusTree *tree) {
    pthread_rwlock_destroy(&tree->lock);
    free_bplustree_nodes(tree->root);
    free(tree);
}
