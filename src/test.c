#include "bptree.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


void writer();

void reader();

void testRW();

void testInsertRuntime(BPlusTree *tree, int num) {
    double startTime, endTime;

    printf("Testing %d inserts on B+ Tree... \n", num);
    startTime = getCurrentTime();
    
    for (int i = 0; i < num; i++) {
        char data[BLOCK_SIZE];
        snprintf(data, BLOCK_SIZE, "Block data %d", i);
        insert(tree, i, data);
    }

    endTime = getCurrentTime();
    printf("Time for %d inserts: %.6f seconds \n", num, endTime - startTime);
}

// /test x y z (x for number of reader, y for number of writer, z for nunber of insert)
int main(int argc, const char *agrv[]) {
    

    // Init Tree and Run different test
    // testinsertRunTime(tree, insertNum);
    return 0;
}