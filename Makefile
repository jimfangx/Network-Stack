# This was my first time writing a makefile so this is likely badly written.
# Guide: https://www.cs.colostate.edu/~cs157/LectureMakefile.pdf

# Compiler
CC := gcc

# Compiler flags
CFLAGS := -Wall -Wextra -I include/

# Source files
SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o)

# Target executable
TARGET := bin/main

# Build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)


# Clean up object files and the executable
clean:
	rm -f $(OBJS) $(TARGET)