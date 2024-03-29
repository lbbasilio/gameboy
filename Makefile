CC = gcc
BIN = bin
SRC = src

FLAGS = -Wall -Wextra -g
VPATH = src:./$(SRC)

SOURCES = $(wildcard $(SRC)/*.c)
OBJECTS = $(patsubst $(SRC)/%.c, $(BIN)/%.o, $(SOURCES)) $(BIN)/vector.o
TARGET = main.exe

$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) $^ -o $@ -lws2_32

$(BIN)/vector.o: $(SRC)/Cup/vector/vector.c
	$(CC) $(FLAGS) -c $< -o $@ -I"$(SRC)"

$(BIN)/%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@ -I"$(SRC)"

clean:
	rm $(BIN)/*.o
	rm $(TARGET)

.PHONY: clean
