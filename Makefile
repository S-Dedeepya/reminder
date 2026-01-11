# Makefile for the GTK Assignment Manager Application

# Compiler and flags
CC = gcc
CFLAGS = `pkg-config --cflags gtk4`
LIBS = `pkg-config --libs gtk4`

# Target executable name
TARGET = assignment_app

# Source files
SOURCES = main_gui.c logic.c

# Object files (derived from source files)
OBJECTS = $(SOURCES:.c=.o)

# Default target: build the application
all: $(TARGET)

# Rule to link the object files into the final executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LIBS)

# Rule to compile a .c file into a .o file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean
