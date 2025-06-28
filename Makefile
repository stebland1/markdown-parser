CC = gcc
CFLAGS = -Wall -Include
BIN_DIR = bin
TARGET = $(BIN_DIR)/mdparser
SRC = src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
