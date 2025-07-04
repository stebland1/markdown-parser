CC = gcc
CFLAGS = -Wall -I./include
BIN_DIR = build

FRONT_MATTER_TARGET = $(BIN_DIR)/md_front_matter
MARKDOWN_TARGET = ${BIN_DIR}/md_markdown

FRONT_MATTER_SRC = src/main_front_matter.c src/front_matter.c src/utils.c
MARKDOWN_SRC = src/main_markdown.c src/markdown.c src/utils.c

all: $(FRONT_MATTER_TARGET) $(MARKDOWN_TARGET)

$(FRONT_MATTER_TARGET): $(FRONT_MATTER_SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(MARKDOWN_TARGET): $(MARKDOWN_SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(FRONT_MATTER_TARGET) $(MARKDOWN_TARGET)
