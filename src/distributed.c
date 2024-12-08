#include "include/distributed.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080

pthread_mutex_t tokenMutex = PTHREAD_MUTEX_INITIALIZER;

// Initialize the distributed node
DistributedNode* initializeNode(int nodeId, int totalNodes, bool initialToken) {
    DistributedNode* node = (DistributedNode*)malloc(sizeof(DistributedNode));
    node->nodeId = nodeId;
    node->totalNodes = totalNodes;
    node->hasToken = initialToken;
    node->nextNode = (nodeId + 1) % totalNodes; // Simple ring topology
    return node;
}

// Request the token
void requestToken(DistributedNode* node) {
    pthread_mutex_lock(&tokenMutex);
    while (!node->hasToken) {
        printf("Node %d: Waiting for token...\n", node->nodeId);
        pthread_mutex_unlock(&tokenMutex);
        sleep(1);
        pthread_mutex_lock(&tokenMutex);
    }
    printf("Node %d: Acquired token!\n", node->nodeId);
    pthread_mutex_unlock(&tokenMutex);
}

// Release the token
void releaseToken(DistributedNode* node) {
    pthread_mutex_lock(&tokenMutex);
    node->hasToken = false;

    // Send token to the next node
    printf("Node %d: Passing token to Node %d...\n", node->nodeId, node->nextNode);
    // For simplicity, this simulation directly updates the next node's state
    // In practice, you'd use a network message to notify the next node
    node->hasToken = false;

    pthread_mutex_unlock(&tokenMutex);
}

// Listen for requests (simulated in this example)
void listenForRequests(DistributedNode* node) {
    printf("Node %d: Listening for token requests...\n", node->nodeId);
    // In practice, this would involve setting up a socket server to listen for requests
    while (1) {
        sleep(5); // Simulate listening
        printf("Node %d: Simulating token passing...\n", node->nodeId);
    }
}
