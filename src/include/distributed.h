#ifndef DISTRIBUTED_H
#define DISTRIBUTED_H

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

// Node-specific information
typedef struct {
    int nodeId;
    int totalNodes; // Total nodes in the distributed system
    bool hasToken;  // Does this node currently hold the token?
    int nextNode;   // ID of the next node to pass the token
} DistributedNode;

// Function declarations
DistributedNode* initializeNode(int nodeId, int totalNodes, bool initialToken);
void requestToken(DistributedNode* node);
void releaseToken(DistributedNode* node);
void listenForRequests(DistributedNode* node);

#endif
