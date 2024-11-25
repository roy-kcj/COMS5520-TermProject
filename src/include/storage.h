#ifndef STORAGE_H
#define STORAGE_H

#define MAX_FILENAME_LENGTH 256

// Prototypes for storage operations
void write_to_file(const char *filename, const char *data);
char* read_from_file(const char *filename);

#endif
