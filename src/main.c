#include "bptree.h"
#include "fat32.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Helper function to print file information
void print_file_info(const char* operation, const char* filename, FAT32_Entry* entry) {
    printf("%s: %s\n", operation, filename);
    if (entry) {
        printf("  Size: %u bytes\n", entry->fileSize);
        printf("  Created: %s", ctime(&entry->creationTime));
        printf("  Modified: %s", ctime(&entry->modificationTime));
        printf("  Start Cluster: %u\n", entry->startCluster);
    }
    printf("\n");
}

int main() {
    // Initialize FAT32 file system (1MB size)
    printf("Initializing FAT32 file system...\n");
    FAT32_FileSystem* fs = fat32_init(1024 * 1024);
    if (!fs) {
        printf("Failed to initialize FAT32 file system\n");
        return 1;
    }

    // Initialize B+ Tree for directory indexing
    printf("Initializing B+ Tree index...\n");
    BPTree* tree = initializeBPTree(fs);
    if (!tree) {
        printf("Failed to initialize B+ Tree\n");
        fat32_cleanup(fs);
        return 1;
    }

    // Test file operations
    printf("\n=== Testing File Operations ===\n\n");

    // Create and insert files
    const char* filenames[] = {"document.txt", "image.jpg", "data.bin"};
    const char* contents[] = {
        "This is a test document.",
        "Binary image data would go here.",
        "Some binary data content."
    };

    // Insert files
    printf("Creating and inserting files...\n");
    for (int i = 0; i < 3; i++) {
        // Create FAT32 entry
        FAT32_Entry* entry = create_file_entry(fs, filenames[i], strlen(contents[i]));
        if (!entry) {
            printf("Failed to create entry for %s\n", filenames[i]);
            continue;
        }

        // Write content
        if (!fat32_write(fs, entry, contents[i], strlen(contents[i]))) {
            printf("Failed to write content to %s\n", filenames[i]);
            continue;
        }

        // Index in B+ Tree
        insert(tree, filenames[i], entry);
        print_file_info("Created", filenames[i], entry);
    }

    // Search for files
    printf("\n=== Testing File Search ===\n\n");
    for (int i = 0; i < 3; i++) {
        FAT32_Entry* entry = search(tree, filenames[i]);
        if (entry) {
            void* content = fat32_read(fs, entry);
            if (content) {
                print_file_info("Found", filenames[i], entry);
                printf("Content: %.*s\n\n", (int)entry->fileSize, (char*)content);
                free(content);
            }
        } else {
            printf("File not found: %s\n\n", filenames[i]);
        }
    }

    // Update a file
    printf("=== Testing File Update ===\n\n");
    const char* update_content = "This is updated content.";
    FAT32_Entry* entry = search(tree, filenames[0]);
    if (entry) {
        fat32_write(fs, entry, update_content, strlen(update_content));
        print_file_info("Updated", filenames[0], entry);
    }

    // Delete a file
    printf("=== Testing File Deletion ===\n\n");
    entry = search(tree, filenames[2]);
    if (entry) {
        fat32_delete(fs, entry);
        delete(tree, filenames[2]);
        printf("Deleted: %s\n\n", filenames[2]);
    }

    // Performance test
    printf("=== Performance Test ===\n\n");
    clock_t start = clock();
    for (int i = 0; i < 1000; i++) {
        char filename[20];
        sprintf(filename, "test%d.txt", i);
        entry = search(tree, filename);
    }
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;
    printf("Time spent searching 1000 files: %.2f ms\n\n", time_spent);

    // Cleanup
    printf("=== Cleaning Up ===\n");
    destroyBPTree(tree);
    fat32_cleanup(fs);
    printf("Cleanup completed successfully\n");

    return 0;
}
