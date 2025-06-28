CC = gcc
CFLAGS = -Wall -I./include
BIN_DIR = build
TARGET = $(BIN_DIR)/mdparser
SRC = src/main.c src/utils.c

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
