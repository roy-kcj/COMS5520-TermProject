#include "include/bptree.h"
#include "include/storage.h"
#include "include/fat32.h"
#include <stdio.h>
#include <time.h>

int main() {
    BPlusTree *tree = initialize_bplustree();

    // Test B+ Tree
    bplustree_insert(tree, 1, "file1.txt");
    bplustree_insert(tree, 2, "file2.txt");
    char *result = (char *)bplustree_search(tree, 2);
    if (result) {
        printf("Found: %s\n", result);
    }

    bplustree_delete(tree, 1);

    // Test FAT32
    fat32_insert("fat32_file.txt", "FAT32 data");
    char *fat32_result = fat32_search("fat32_file.txt");
    if (fat32_result) {
        printf("FAT32 Found: %s\n", fat32_result);
        free(fat32_result);
    }
    fat32_delete("fat32_file.txt");

    // Cleanup
    free_bplustree(tree);
    return 0;
}
