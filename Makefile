CC = gcc
BIN = bin
SRC = src

FLAGS = -Wall -Wextra -g
VPATH = src:./$(SRC)

SOURCES = $(wildcard $(SRC)/*.c)
OBJECTS = $(patsubst $(SRC)/%.c, $(BIN)/%.o, $(SOURCES))
TARGET = main.exe

$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) $^ -o $@ -lws2_32

$(BIN)/%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@ -I"$(SRC)"

clean:
	rm $(TARGET)
	rm $(BIN)/*.o

.PHONY: clean
