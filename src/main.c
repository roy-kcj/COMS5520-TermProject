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
    char input[100]; 
    while (1) {
        printf("Enter commands (type 'exit' to quit)\n");
        printf("Commands: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        // Exit
        if (strcmp(input, "exit") == 0) {
            printf("Exiting ....\n");
            break;
        }

        switch (input[0]) {
            case 't':
                // Initialize FAT32 file system (1MB size)
                printf("Initializing FAT32 file system...\n");
                FAT32_FileSystem* fs = fat32_init(1024 * 1024);
                if (!fs) {
                    printf("Failed to initialize FAT32 file system\n");
                    continue;
                }

                // Initialize B+ Tree for directory indexing
                printf("Initializing B+ Tree index...\n");
                BPTree* tree = initializeBPTree(fs);
                if (!tree) {
                    printf("Failed to initialize B+ Tree\n");
                    fat32_cleanup(fs);
                    continue;
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

<<<<<<< Updated upstream
=======
                printf("Test Case 1: Bulk Insert Performance\n");
                if (!fs || !tree || !tree->root) {
                    printf("File system or B+ Tree not properly initialized\n");
                    return;
                }

                clock_t start = clock();
                for (int i = 0; i < 1000; i++) {
                    char filename[32];
                    char content[64];
                    
                    if (snprintf(filename, sizeof(filename), "test_file_%d.txt", i) >= sizeof(filename)) {
                        printf("Filename truncated\n");
                        continue;
                    }
                    
                    if (snprintf(content, sizeof(content), "Content for file %d", i) >= sizeof(content)) {
                        printf("Content truncated\n");
                        continue;
                    }
                    
                    FAT32_Entry* entry = create_file_entry(fs, filename, strlen(content));
                    if (!entry) {
                        printf("Failed to create entry for file: %s\n", filename);
                        continue;
                    }
                    
                    if (!fat32_write(fs, entry, content, strlen(content))) {
                        printf("Failed to write content for file: %s\n", filename);
                        free(entry);
                        continue;
                    }
                    
                    insert(tree, filename, entry);
                }
                printf("Time to insert 1000 files: %.2f ms\n\n", 
                    (double)(clock() - start) / CLOCKS_PER_SEC * 1000);

                // Random Access Test
                printf("Test Case 3: Random Access Performance\n");
                start = clock();
                for (int i = 0; i < 1000; i++) {
                    char filename[32];
                    sprintf(filename, "test_file_%d.txt", rand() % 1000);
                    FAT32_Entry* entry = search(tree, filename);
                    if (entry) {
                        void* content = fat32_read(fs, entry);
                        free(content);
                    }
                }
                printf("Time for 1000 random accesses: %.2f ms\n\n", 
                       (double)(clock() - start) / CLOCKS_PER_SEC * 1000);

                // Sequential Update Test
                printf("Test Case 3: Sequential Update Performance\n");
                start = clock();
                for (int i = 0; i < 100; i++) {
                    char old_name[32], new_name[32];
                    sprintf(old_name, "test_file_%d.txt", i);
                    sprintf(new_name, "updated_file_%d.txt", i);
                    FAT32_Entry* entry = search(tree, old_name);
                    if (entry) {
                        update(tree, old_name, new_name, entry);
                    }
                }
                printf("Time to update 100 files: %.2f ms\n\n", 
                       (double)(clock() - start) / CLOCKS_PER_SEC * 1000);

                // Mixed Operations Test
                printf("Test Case 4: Mixed Operations Performance\n");
                start = clock();
                for (int i = 0; i < 100; i++) {
                    // Insert
                    char filename[32];
                    sprintf(filename, "mixed_file_%d.txt", i);
                    FAT32_Entry* entry = create_file_entry(fs, filename, 64);
                    insert(tree, filename, entry);
            
                    // Search
                    entry = search(tree, filename);
            
                    // Update
                    if (entry && i % 2 == 0) {
                        char new_name[32];
                        sprintf(new_name, "mixed_updated_%d.txt", i);
                        update(tree, filename, new_name, entry);
                    }
            
                    // Delete
                    if (i % 3 == 0) {
                        delete(tree, filename);
                    }
                }
                printf("Time for 100 mixed operations: %.2f ms\n\n", 
                       (double)(clock() - start) / CLOCKS_PER_SEC * 1000);

                // Large File Test
                printf("Test Case 5: Large File Handling\n");
                start = clock();
                char* large_content = (char*)malloc(1024 * 1024); // 1MB content
                memset(large_content, 'X', 1024 * 1024);
                FAT32_Entry* large_entry = create_file_entry(fs, "large_file.txt", 1024 * 1024);
                fat32_write(fs, large_entry, large_content, 1024 * 1024);
                insert(tree, "large_file.txt", large_entry);
                printf("Time to handle 1MB file: %.2f ms\n\n", 
                       (double)(clock() - start) / CLOCKS_PER_SEC * 1000);
                free(large_content);

                // Sequential vs Random Access
                printf("Test Case 8: Sequential vs Random Access\n");
                start = clock();
                
                // Sequential access
                for (int i = 0; i < 1000; i++) {
                    char filename[32];
                    sprintf(filename, "seq_file_%d.txt", i);
                    FAT32_Entry* entry = search(tree, filename);
                }
                double seq_time = (double)(clock() - start) / CLOCKS_PER_SEC * 1000;
                
                // Random access
                start = clock();
                for (int i = 0; i < 1000; i++) {
                    char filename[32];
                    sprintf(filename, "seq_file_%d.txt", rand() % 1000);
                    FAT32_Entry* entry = search(tree, filename);
                }
                double rand_time = (double)(clock() - start) / CLOCKS_PER_SEC * 1000;
                
                printf("Sequential access time: %.2f ms\n", seq_time);
                printf("Random access time: %.2f ms\n\n", rand_time);
                
>>>>>>> Stashed changes
                // Performance test
                printf("=== Performance Test ===\n\n");
                start = clock();
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
                break;

            case 's':
            
                break;
        
            default:
                break;
        }
    }
    
    

    return 0;
}
