CC = gcc
CFLAGS = -Wall -g -O0 -I./include -fsanitize=address -fno-omit-frame-pointer
BIN_DIR = build

FRONT_MATTER_TARGET = $(BIN_DIR)/md_front_matter
MARKDOWN_TARGET = ${BIN_DIR}/md_markdown
TEST_TARGET = $(BIN_DIR)/tests

RENDERER_SRC := $(wildcard src/renderer/*.c)
UTILS_SRC := $(wildcard src/utils/*.c)
BLOCKS_SRC := $(wildcard src/blocks/*.c)
INLINE_SRC := $(wildcard src/inline/*.c)
TESTS_SRC := tests/main.c tests/test.c $(wildcard tests/test_*.c) $(BLOCKS_SRC) $(INLINE_SRC) $(UTILS_SRC) src/token.c src/context.c src/parser.c $(RENDERER_SRC)

FRONT_MATTER_SRC = src/main/front_matter.c \
									 src/front_matter/parser.c \
									 src/utils/utils.c \
									 src/front_matter/entries.c \
									 src/front_matter/list.c
MARKDOWN_SRC = src/main/markdown.c \
							 src/parser.c \
							 src/context.c \
							 src/token.c \
							 $(RENDERER_SRC) \
							 $(UTILS_SRC) \
							 $(BLOCKS_SRC) \
							 $(INLINE_SRC)

all: $(FRONT_MATTER_TARGET) $(MARKDOWN_TARGET)

$(FRONT_MATTER_TARGET): $(FRONT_MATTER_SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(MARKDOWN_TARGET): $(MARKDOWN_SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_TARGET): $(TESTS_SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

test: $(TEST_TARGET)
	./$(TEST_TARGET)

test-%: tests/test_%.c
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o build/$* $^ tests/test.c tests/main.c $(UTILS_SRC) $(INLINE_SRC) src/token.c
	./build/$*

clean:
	rm -f $(FRONT_MATTER_TARGET) $(MARKDOWN_TARGET) $(TEST_TARGET) build/test-*
