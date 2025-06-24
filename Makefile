# Defines build rules for compiling the project

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2 -Iinclude
LDFLAGS = -pthread

# Directories
SRCDIR = src
INCDIR = include
TESTDIR = tests 
BUILDDIR = build

# Source files
SOURCES = $(SRCDIR)/kvstore.c $(SRCDIR)/hash_table.c $(SRCDIR)/persistence.c $(SRCDIR)/error.c
MAIN_SRC = $(SRCDIR)/main.c
TEST_SRC = $(TESTDIR)/test.c 

# Object files
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
MAIN_OBJ = $(BUILDDIR)/main.o 
TEST_OBJ = $(BUILDDIR)/test.o 

# Executables
TARGET = kvstore 
TEST_TARGET = test_kvstore 

# Default target 
all: $(TARGET) 

# Create build directory 
$(BUILDDIR): 
	mkdir -p $(BUILDDIR)

# Compile source files to object files 
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@ 

# Build main executable 
$(TARGET): $(OBJECTS) $(MAIN_OBJ)
	$(CC) $(OBJECTS) $(MAIN_OBJ) -o $(TARGET) $(LDFLAGS)

# Build test executables
$(TEST_TARGET): $(OBJECTS) $(TEST_OBJ)
	$(CC) $(OBJECTS) $(TEST_OBJ) -o $(TEST_TARGET) $(LDFLAGS)

# Run tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Run with valgrind for memeory leak detection
valgrind: $(TEST_TARGET) 
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TEST_TARGET)

# Run the main program 
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean: 
	rm -rf $(BUILDDIR) $(TARGET) $(TEST_TARGET) *.bin

# Install (copy to /usr/local/bin)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin

# Uninstall 
uninstall: 
	rm -f /usr/local/bin/$(TARGET)

#help
help:
	@echo "Available targets:"
	@echo "  all      - Build the main executable (default)"
	@echo "  test     - Build and run tests"
	@echo "  valgrind - Run tests with memory leak detection"
	@echo "  run      - Build and run the main program"
	@echo "  clean    - Remove build artifacts"
	@echo "  install  - Install to /usr/local/bin"
	@echo "  help     - Show this help message"

# Phony targets
.PHONY: all test valgrind run clean install uninstall help

