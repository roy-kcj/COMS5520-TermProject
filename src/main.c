#include "include/bptree.h"
#include "include/fat32.h"
#include "include/distributed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void performSequentialRAOperations()
{
    // Initialize FAT32 file system (1MB size)
    printf("Initializing FAT32 file system...\n");
    FAT32_FileSystem *fs = fat32_init(1024 * 1024);
    if (!fs)
    {
        printf("Failed to initialize FAT32 file system\n");
        return;
    }

    // Initialize B+ Tree for directory indexing
    printf("Initializing B+ Tree index...\n");
    BPTree *tree = initializeBPTree(fs);
    if (!tree)
    {
        printf("Failed to initialize B+ Tree\n");
        fat32_cleanup(fs);
        return;
    }

    // Sequential vs Random Access
    printf("Test Case 1: Sequential vs Random Access\n");
    clock_t start = clock();

    // Sequential access
    for (int i = 0; i < 1000; i++)
    {
        char filename[32];
        sprintf(filename, "seq_file_%d.txt", i);
        FAT32_Entry *entry = search(tree, filename);
    }
    double seq_time = (double)(clock() - start) / CLOCKS_PER_SEC * 1000;

    // Random access
    start = clock();
    for (int i = 0; i < 1000; i++)
    {
        char filename[32];
        sprintf(filename, "seq_file_%d.txt", rand() % 1000);
        FAT32_Entry *entry = search(tree, filename);
    }
    double rand_time = (double)(clock() - start) / CLOCKS_PER_SEC * 1000;

    printf("Sequential access time: %.2f ms\n", seq_time);
    printf("Random access time: %.2f ms\n\n", rand_time);

    // Cleanup
    printf("=== Cleaning Up ===\n");
    destroyBPTree(tree);
    fat32_cleanup(fs);
    printf("Cleanup completed successfully\n");
    return;
}

void performCRUDTestOperations()
{
    // Initialize FAT32 file system (1MB size)
    printf("Initializing FAT32 file system...\n");
    FAT32_FileSystem *fs = fat32_init(1024 * 1024);
    if (!fs)
    {
        printf("Failed to initialize FAT32 file system\n");
        return;
    }

    // Initialize B+ Tree for directory indexing
    printf("Initializing B+ Tree index...\n");
    BPTree *tree = initializeBPTree(fs);
    if (!tree)
    {
        printf("Failed to initialize B+ Tree\n");
        fat32_cleanup(fs);
        return;
    }

    // Test file operations
    printf("\n=== Testing File Operations ===\n\n");

    // Create and insert files
    const char *filenames[] = {"document.txt", "image.jpg", "data.bin"};
    const char *contents[] = {
        "This is a test document.",
        "Binary image data would go here.",
        "Some binary data content."};

    // Insert files
    printf("Creating and inserting files...\n");
    for (int i = 0; i < 3; i++)
    {
        // Create FAT32 entry
        FAT32_Entry *entry = create_file_entry(fs, filenames[i], strlen(contents[i]));
        if (!entry)
        {
            printf("Failed to create entry for %s\n", filenames[i]);
            continue;
        }

        // Write content
        if (!fat32_write(fs, entry, contents[i], strlen(contents[i])))
        {
            printf("Failed to write content to %s\n", filenames[i]);
            continue;
        }

        // Index in B+ Tree
        insert(tree, filenames[i], entry);
        print_file_info("Created", filenames[i], entry);
    }

    // Search for files
    printf("\n=== Testing File Search ===\n\n");
    for (int i = 0; i < 3; i++)
    {
        FAT32_Entry *entry = search(tree, filenames[i]);
        if (entry)
        {
            void *content = fat32_read(fs, entry);
            if (content)
            {
                print_file_info("Found", filenames[i], entry);
                printf("Content: %.*s\n\n", (int)entry->fileSize, (char *)content);
                free(content);
            }
        }
        else
        {
            printf("File not found: %s\n\n", filenames[i]);
        }
    }

    // Update a file
    printf("=== Testing File Update ===\n\n");
    const char *update_content = "This is updated content.";
    FAT32_Entry *entry = search(tree, filenames[0]);
    if (entry)
    {
        fat32_write(fs, entry, update_content, strlen(update_content));
        print_file_info("Updated", filenames[0], entry);
    }

    // Delete a file
    printf("=== Testing File Deletion ===\n\n");
    entry = search(tree, filenames[2]);
    if (entry)
    {
        fat32_delete(fs, entry);
        delete (tree, filenames[2]);
        printf("Deleted: %s\n\n", filenames[2]);
    }

    // Performance test
    printf("=== Performance Test ===\n\n");
    clock_t start = clock();
    for (int i = 0; i < 1000; i++)
    {
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
}

void performBulkTestOperations(FAT32_FileSystem *fs, BPTree *tree, int size)
{
    // Test Case 1: Bulk Insert Test
    printf("=== Test Case 1: Bulk Insert Performance ===\n");
    if (fs && tree && tree->root)
    {
        clock_t start = clock();
        for (int i = 0; i < size; i++)
        {
            char filename[32] = {0};
            char content[64] = {0};

            if (snprintf(filename, sizeof(filename), "test_file_%d.txt", i) >= sizeof(filename))
            {
                printf("Filename truncated, skipping...\n");
                continue;
            }

            if (snprintf(content, sizeof(content), "Content for file %d", i) >= sizeof(content))
            {
                printf("Content truncated, skipping...\n");
                continue;
            }

            FAT32_Entry *entry = create_file_entry(fs, filename, strlen(content));
            if (!entry)
            {
                printf("Failed to create entry for: %s\n", filename);
                continue;
            }

            if (!fat32_write(fs, entry, content, strlen(content)))
            {
                printf("Failed to write content for: %s\n", filename);
                free(entry);
                continue;
            }

            insert(tree, filename, entry);
        }
        printf("Time to insert %d files: %.2f ms\n\n", size,
               (double)(clock() - start) / CLOCKS_PER_SEC * 1000);
    }

    // Test Case 2: Random Access Test
    printf("=== Test Case 2: Random Access Performance ===\n");
    if (fs && tree && tree->root)
    {
        clock_t start = clock();
        for (int i = 0; i < size; i++)
        {
            char filename[32] = {0};
            int random_index = rand() % size;

            if (snprintf(filename, sizeof(filename), "test_file_%d.txt", random_index) < sizeof(filename))
            {
                FAT32_Entry *entry = search(tree, filename);
                if (entry)
                {
                    void *content = fat32_read(fs, entry);
                    if (content)
                    {
                        free(content);
                    }
                }
            }
        }
        printf("Time for %d random accesses: %.2f ms\n\n", size,
               (double)(clock() - start) / CLOCKS_PER_SEC * 1000);
    }

    // Test Case 3: Large File Test
    printf("=== Test Case 3: Large File Handling ===\n");
    if (fs && tree && tree->root)
    {
        clock_t start = clock();
        char *large_content = (char *)malloc(512 * 1024); // 512KB instead of 1MB for stability
        if (large_content)
        {
            memset(large_content, 'X', 512 * 1024);
            FAT32_Entry *large_entry = create_file_entry(fs, "large_file.txt", 512 * 1024);
            if (large_entry)
            {
                if (fat32_write(fs, large_entry, large_content, 512 * 1024))
                {
                    insert(tree, "large_file.txt", large_entry);
                }
            }
            free(large_content);
        }
        printf("Time to handle 512KB file: %.2f ms\n\n",
               (double)(clock() - start) / CLOCKS_PER_SEC * 1000);
    }
}

void *performCriticalOperations(void *arg)
{
    DistributedNode *node = (DistributedNode *)arg;
    BPTree *tree = node->sharedTree;       // Access shared B+Tree
    FAT32_FileSystem *fs = node->sharedFS; // Access shared FAT32 file system
    PriorityQueue *queue = node->queue;

    while (1)
    {
        // Dequeue the highest-priority task
        Task task = dequeue(queue);

        // Acquire token for distributed mutual exclusion
        requestToken(node);

        if (strcmp(task.operation, "insert") == 0)
        {
            // Perform insert operation
            char filename[32];
            snprintf(filename, sizeof(filename), "node_%d_file_%d.txt", node->nodeId, task.processID);
            FAT32_Entry *entry = create_file_entry(fs, filename, 64);
            if (entry)
            {
                insert_dme(tree, filename, entry, node);
                printf("Node %d: Inserted %s into B+Tree\n", node->nodeId, filename);
            }
        }
        else if (strcmp(task.operation, "search") == 0)
        {
            // Perform search operation
            char filename[32];
            snprintf(filename, sizeof(filename), "node_%d_file_%d.txt", node->nodeId, task.processID);
            FAT32_Entry *result = search(tree, filename);
            if (result)
            {
                printf("Node %d: Found %s in B+Tree\n", node->nodeId, filename);
            }
            else
            {
                printf("Node %d: %s not found in B+Tree\n", node->nodeId, filename);
            }
        }
        else if (strcmp(task.operation, "delete") == 0)
        {
            // Perform delete operation
            char filename[32];
            snprintf(filename, sizeof(filename), "node_%d_file_%d.txt", node->nodeId, task.processID);
            delete_dme(tree, filename, node);
            printf("Node %d: Deleted %s from B+Tree\n", node->nodeId, filename);
        }

        // Release the token after finishing the task
        releaseToken(node);
    }

    return NULL;
}

// Helper function to print file information
void print_file_info(const char *operation, const char *filename, FAT32_Entry *entry)
{
    printf("%s: %s\n", operation, filename);
    if (entry)
    {
        printf("  Size: %u bytes\n", entry->fileSize);
        printf("  Created: %s", ctime(&entry->creationTime));
        printf("  Modified: %s", ctime(&entry->modificationTime));
        printf("  Start Cluster: %u\n", entry->startCluster);
    }
    printf("\n");
}

int main()
{
    char input[100];
    while (1)
    {
        printf("Enter commands (type 'exit' to quit)\n");
        printf("Commands: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        // Exit
        if (strcmp(input, "exit") == 0)
        {
            printf("Exiting ....\n");
            break;
        }

        switch (input[0])
        {
        case 'b':
        {
            // Initialize FAT32 file system (1MB size)
            printf("Initializing FAT32 file system...\n");
            FAT32_FileSystem *fs = fat32_init(1024 * 1024);
            if (!fs)
            {
                printf("Failed to initialize FAT32 file system\n");
                continue;
            }

            // Initialize B+ Tree for directory indexing
            printf("Initializing B+ Tree index...\n");
            BPTree *tree = initializeBPTree(fs);
            if (!tree)
            {
                printf("Failed to initialize B+ Tree\n");
                fat32_cleanup(fs);
                continue;
            }

            const int sample[] = {100, 200, 500, 1000, 2000};

            for (int i = 0; i < sizeof(sample) / sizeof(sample[0]); i++)
                performBulkTestOperations(fs, tree, sample[i]);

            // Cleanup
            printf("=== Cleaning Up ===\n");
            destroyBPTree(tree);
            fat32_cleanup(fs);
            printf("Cleanup completed successfully\n");

            break;
        }
        case 'c':
        {
            performCRUDTestOperations();
            break;
        }
        case 'd':
        {
            printf("\n=== Distributed B+Tree Test ===\n");

            // Initialize FAT32 file system (1MB size)
            FAT32_FileSystem *fs = fat32_init(1024 * 1024);
            if (!fs)
            {
                printf("Failed to initialize FAT32 file system\n");
                break;
            }

            // Initialize B+ Tree for directory indexing
            BPTree *tree = initializeBPTree(fs);
            if (!tree)
            {
                printf("Failed to initialize B+ Tree\n");
                fat32_cleanup(fs);
                break;
            }

            int totalNodes = 3; // Example: 3 distributed nodes
            DistributedNode* nodes[totalNodes];

            for (int i = 0; i < totalNodes; i++) {
                nodes[i] = initializeNode(i, totalNodes, i == 0, tree, fs); // Node 0 starts with the token
            }
            // Create tasks and assign them to specific nodes
            for (int i = 0; i < 30; i++)
            {
                Task task;
                task.processID = i;
                if (i % 3 == 0)
                {
                    strcpy(task.operation, "insert");
                }
                else if (i % 3 == 1)
                {
                    strcpy(task.operation, "search");
                }
                else
                {
                    strcpy(task.operation, "delete");
                }
                task.timestamp = time(NULL) + i;

                // Assign the task to a specific node
                int nodeIndex = i % totalNodes;
                enqueue(nodes[nodeIndex]->queue, task);
            }

            // Start threads for each distributed node
            pthread_t threads[totalNodes];
            for (int i = 0; i < totalNodes; i++)
            {
                pthread_create(&threads[i], NULL, performCriticalOperations, (void *)nodes[i]);
            }

            // Let threads run for a while
            sleep(2);

            // Cleanup
            for (int i = 0; i < totalNodes; i++)
            {
                pthread_cancel(threads[i]); // Terminate threads
                pthread_join(threads[i], NULL);
                free(nodes[i]->queue);
                free(nodes[i]);
            }

            // Clean up B+Tree and FAT32
            destroyBPTree(tree);
            fat32_cleanup(fs);

            printf("Distributed test completed.\n");
            break;
        }
        case 's':
        {
            performSequentialRAOperations();
            break;
        }
        default:
            break;
        }
    }

    return 0;
}
