# Compiler to be used
CC=gcc
# Compiler flags
CFLAGS=-Iinclude
# Header file dependencies
DEPS = HTTP_Server.h
# Name of the generated executable file
exec = build/server.o
# Source C files in the src directory
sources = $(wildcard src/*.c)
# Object files generated in the build directory
objects = $(patsubst src/%.c, build/%.o, $(sources))
# Additional flags for compilation
flags = -g -Wall -lm -ldl -fPIC -rdynamic -I./include

# Rule to generate the executable file
$(exec): $(objects)
	${CC} $(objects) $(flags) -o $(exec)

# Rule to compile each .c file into .o
build/%.o: src/%.c
	@mkdir -p build
	${CC} -c $(flags) $< -o $@

# Rule to clean the build files
clean:
	-rm -rf build
