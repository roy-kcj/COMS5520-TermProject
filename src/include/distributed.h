#ifndef DISTRIBUTED_H
#define DISTRIBUTED_H

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include "bptree.h"
#include "fat32.h"  

// Maximum number of tasks in the queue
#define MAX_QUEUE_SIZE 100
#define MAX_NODES 100

// Task structure for priority queue
typedef struct {
    int processID;        // ID of the process/thread requesting the token
    char operation[10];   // Type of operation (e.g., "insert", "delete")
    long timestamp;       // Timestamp used for prioritizing tasks
} Task;

// Priority queue for managing tasks
typedef struct {
    Task tasks[MAX_QUEUE_SIZE]; // Array to store tasks
    int size;                   // Current number of tasks
    pthread_mutex_t mutex;      // Mutex for thread-safe operations
    pthread_cond_t cond;        // Condition variable for signaling
} PriorityQueue;

// Distributed node structure
typedef struct {
    int nodeId;               // ID of this node
    int totalNodes;           // Total number of nodes in the system
    bool hasToken;            // Whether this node currently holds the token
    int nextNode;             // ID of the next node in the ring
    PriorityQueue* queue;     // Priority queue for managing tasks
    BPTree* sharedTree;       // Pointer to shared B+Tree
    FAT32_FileSystem* sharedFS; // Pointer to shared FAT32 file system
} DistributedNode;

// Function declarations
DistributedNode* initializeNode(int nodeId, int totalNodes, bool initialToken, BPTree* tree, FAT32_FileSystem* fs);
void requestToken(DistributedNode* node);
void releaseToken(DistributedNode* node);

// Priority queue operations
void enqueue(PriorityQueue* queue, Task task); // Add a task to the priority queue
Task dequeue(PriorityQueue* queue);            // Remove the highest-priority task

#endif // DISTRIBUTED_H
