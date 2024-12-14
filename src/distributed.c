#include "include/distributed.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define MAX_QUEUE_SIZE 100

pthread_mutex_t tokenMutex = PTHREAD_MUTEX_INITIALIZER;

// Initialize the distributed node
DistributedNode* initializeNode(int nodeId, int totalNodes, bool initialToken, BPTree* tree, FAT32_FileSystem* fs) {
    DistributedNode* node = (DistributedNode*)malloc(sizeof(DistributedNode));
    node->nodeId = nodeId;
    node->totalNodes = totalNodes;
    node->hasToken = initialToken;
    node->nextNode = (nodeId + 1) % totalNodes; // Simple ring topology
    node->queue = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    node->queue->size = 0;
    pthread_mutex_init(&node->queue->mutex, NULL);
    pthread_cond_init(&node->queue->cond, NULL);

    // Set shared resources
    node->sharedTree = tree;
    node->sharedFS = fs;

    return node;
}


// Request token with priority
void requestToken(DistributedNode* node) {
    pthread_mutex_lock(&node->queue->mutex);

    // Wait until token is acquired
    while (!node->hasToken || node->queue->size == 0) {
        pthread_cond_wait(&node->queue->cond, &node->queue->mutex);
    }

    printf("Node %d: Acquired token!\n", node->nodeId);
    pthread_mutex_unlock(&node->queue->mutex);
}

// Release token and pass to the next process
void releaseToken(DistributedNode* node) {
    pthread_mutex_lock(&node->queue->mutex);

    printf("Node %d: Releasing token.\n", node->nodeId);
    node->hasToken = false;

    // Notify the next node or highest-priority task
    pthread_cond_broadcast(&node->queue->cond);
    pthread_mutex_unlock(&node->queue->mutex);
}

// Add a task to the priority queue
void enqueue(PriorityQueue* queue, Task task) {
    pthread_mutex_lock(&queue->mutex);

    if (queue->size >= MAX_QUEUE_SIZE) {
        printf("Queue is full!\n");
        pthread_mutex_unlock(&queue->mutex);
        return;
    }

    queue->tasks[queue->size++] = task;

    // Heapify up for priority ordering
    int i = queue->size - 1;
    while (i > 0 && queue->tasks[i].timestamp < queue->tasks[(i - 1) / 2].timestamp) {
        Task temp = queue->tasks[i];
        queue->tasks[i] = queue->tasks[(i - 1) / 2];
        queue->tasks[(i - 1) / 2] = temp;
        i = (i - 1) / 2;
    }

    pthread_cond_signal(&queue->cond); // Signal any waiting threads
    pthread_mutex_unlock(&queue->mutex);
}

// Remove the highest-priority task from the queue
Task dequeue(PriorityQueue* queue) {
    pthread_mutex_lock(&queue->mutex);

    while (queue->size == 0) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    Task top = queue->tasks[0];
    queue->tasks[0] = queue->tasks[--queue->size];

    // Heapify down
    int i = 0;
    while (2 * i + 1 < queue->size) {
        int smallest = 2 * i + 1;
        if (smallest + 1 < queue->size && queue->tasks[smallest + 1].timestamp < queue->tasks[smallest].timestamp) {
            smallest++;
        }
        if (queue->tasks[i].timestamp <= queue->tasks[smallest].timestamp) {
            break;
        }
        Task temp = queue->tasks[i];
        queue->tasks[i] = queue->tasks[smallest];
        queue->tasks[smallest] = temp;
        i = smallest;
    }

    pthread_mutex_unlock(&queue->mutex);
    return top;
}

// // Initialize the distributed node
// DistributedNode* initializeNode(int nodeId, int totalNodes, bool initialToken) {
//     DistributedNode* node = (DistributedNode*)malloc(sizeof(DistributedNode));
//     node->nodeId = nodeId;
//     node->totalNodes = totalNodes;
//     node->hasToken = initialToken;
//     node->nextNode = (nodeId + 1) % totalNodes; // Simple ring topology
//     return node;
// }

// // Request the token
// void requestToken(DistributedNode* node) {
//     pthread_mutex_lock(&tokenMutex);
//     while (!node->hasToken) {
//         printf("Node %d: Waiting for token...\n", node->nodeId);
//         pthread_mutex_unlock(&tokenMutex);
//         sleep(1);
//         pthread_mutex_lock(&tokenMutex);
//     }
//     printf("Node %d: Acquired token!\n", node->nodeId);
//     pthread_mutex_unlock(&tokenMutex);
// }

// // Release the token
// void releaseToken(DistributedNode* node) {
//     pthread_mutex_lock(&tokenMutex);
//     node->hasToken = false;

//     // Send token to the next node
//     printf("Node %d: Passing token to Node %d...\n", node->nodeId, node->nextNode);
//     // For simplicity, this simulation directly updates the next node's state
//     // In practice, you'd use a network message to notify the next node
//     node->hasToken = false;

//     pthread_mutex_unlock(&tokenMutex);
// }

// Listen for requests (simulated in this example)
void listenForRequests(DistributedNode* node) {
    printf("Node %d: Listening for token requests...\n", node->nodeId);

    // Implemetation extended for when websockets is used for receiving actual request from distributed environments
    // Future Work
}
