CC = gcc
CFLAGS = -I../include -Iunity/src -Wall -g -DTEST_UNIT  # Menambahkan definisi TEST_UNIT

SRC = unit_test.c unity/src/unity.c \
      ../src/HTTP_Server.c \
      ../src/Routes.c \
      ../src/Response.c \
      ../src/Parameters.c \
      ../src/Server.c

OBJ_DIR = build
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.c=.o)))

EXEC = build/unit_test

VPATH = unity/src:../src

# Menyusun dan menjalankan unit test
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $(EXEC) $(OBJ)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run_tests: $(EXEC)
	./$(EXEC)

clean:
	rm -rf $(OBJ_DIR) $(EXEC)

.PHONY: all run_tests clean
