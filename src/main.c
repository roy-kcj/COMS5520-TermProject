#include "bptree.h"
#include <stdio.h>
#include <string.h>

int main() {
    // Initialize B+ Tree
    BPTree* tree = initializeBPTree();
    
    // Test data
    const char* files[] = {"file1.txt", "file2.txt", "file3.txt"};
    const char* data[] = {"Data 1", "Data 2", "Data 3"};
    
    // Insert test
    printf("Inserting files...\n");
    for (int i = 0; i < 3; i++) {
        char* value = strdup(data[i]);
        insert(tree, files[i], value);
        printf("Inserted: %s\n", files[i]);
    }
    
    // Search test
    printf("\nSearching for files...\n");
    for (int i = 0; i < 3; i++) {
        void* result = search(tree, files[i]);
        if (result) {
            printf("Found %s: %s\n", files[i], (char*)result);
        }
    }
    
    // Update test
    printf("\nUpdating file1.txt to newfile.txt...\n");
    update(tree, "file1.txt", "newfile.txt", strdup("Updated Data"));
    
    // Delete test
    printf("\nDeleting file3.txt...\n");
    delete(tree, "file3.txt");
    
    // Cleanup
    destroyBPTree(tree);
    printf("\nB+ Tree cleaned up successfully\n");
    
    return 0;
}
