CC = gcc
CFLAGS = -I../include -Iunity/src -Wall -g -DTEST_UNIT  # Menambahkan definisi TEST_UNIT

SRC = unit_test.c unity/src/unity.c \
      ../src/HTTP_Server.c \
      ../src/Routes.c \
      ../src/Response.c \
      ../src/Parameters.c \
      ../src/Server.c

OBJ = $(SRC:.c=.o)

EXEC = unit_test

# Menyusun dan menjalankan unit test
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $(EXEC) $(OBJ)

run_tests: $(EXEC)
	./$(EXEC)

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all run_tests clean
