CC = gcc
CFLAGS = -Wall -g -O0 -I./include
BIN_DIR = build

FRONT_MATTER_TARGET = $(BIN_DIR)/md_front_matter
MARKDOWN_TARGET = ${BIN_DIR}/md_markdown

UTILS_SRC := $(wildcard src/utils/*.c)
BLOCKS_SRC := $(wildcard src/blocks/*.c)
INLINE_SRC := $(wildcard src/inline/*.c)

FRONT_MATTER_SRC = src/main/front_matter.c src/front_matter.c src/utils/utils.c
MARKDOWN_SRC = src/main/markdown.c \
							 src/parser.c \
							 src/token.c \
							 src/inline.c \
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

clean:
	rm -f $(FRONT_MATTER_TARGET) $(MARKDOWN_TARGET)
