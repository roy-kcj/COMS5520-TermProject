#include "include/storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write_to_file(const char *filename, const char *data) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file");
        return;
    }
    fprintf(file, "%s", data);
    fclose(file);
}

char* read_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(size + 1);
    fread(buffer, 1, size, file);
    buffer[size] = '\0';

    fclose(file);
    return buffer;
}
