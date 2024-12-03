CC=gcc
CFLAGS=-Iinclude
DEPS = HTTP_Server.h
exec = build/server.o
sources = $(wildcard src/*.c)
objects = $(patsubst src/%.c, build/%.o, $(sources))
flags = -g -Wall -lm -ldl -fPIC -rdynamic -I./include

$(exec): $(objects)
	${CC} $(objects) $(flags) -o $(exec)

build/%.o: src/%.c
	@mkdir -p build
	${CC} -c $(flags) $< -o $@

clean:
	-rm -rf build
