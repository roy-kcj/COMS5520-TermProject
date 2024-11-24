#ifndef BPTREE_H
#define BPTREE_H

#define BLOCK_SIZE  512
#define MAX_KEYS    25

typedef struct Block {
    int id;
    char data[BLOCK_SIZE];
} Block;

typedef struct BPlusTreeNode {
    int keys[MAX_KEYS]; // array of keys
    Block **blocks; // data block (only relevant to leaf nodes)
    struct BPlusTreeNode **children; // array of children pointers 
    int n;  // current num of keys
    bool leaf;  // boolean to check if node is leaf
    struct BPlusTreeNode *next; // next leaf node
} BPlusTreeNode;

typedef struct BPlusTree {
    BPlusTreeNode *root;
    int t; // minimum degree
} BPlusTree;

BPlusTree *initializeTree(int fanout);
BPlusTreeNode *createNode(int t, bool leaf);

void splitNode(BPlusTreeNode *parent, int i, BPlusTreeNode *child);
void insertNonFull(BPlusTree *tree, const char *key, const char *data);
void insert(BPlusTree *tree, const char *key, const char *data);

void merge(BPlusTreeNode *node, int idx);
void deleteNode(BPlusTree *tree, const char*key);
void readBlock(BPlusTree *tree, const char *key, char *buffer);
void traverse(BPlusTreeNode *node);
void printTree(BPlusTreeNode *root);

#endif