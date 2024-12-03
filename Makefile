# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -I./src/include -pthread

# Directories
SRC_DIR = src
INCLUDE_DIR = src/include
OBJ_DIR = obj
BIN_DIR = bin

# Source files and object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Output executable
TARGET = $(BIN_DIR)/bptree_test

# Default target
all: setup $(TARGET)

# Create output directories
setup:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Compile the target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile individual object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Run the program
run: all
	@./$(TARGET)

# Phony targets
.PHONY: all clean run setup
