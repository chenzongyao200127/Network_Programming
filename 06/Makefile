# Makefile for the 'tiny' project

# Compiler and compiler flags
CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS = -lpthread -lrt

# Target executable name
TARGET = tiny

# Source files
SRCS = tiny.c csapp.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# To create object files
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Clean up
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: clean
