CC = gcc
CFLAGS = -Wall -O2 -std=c11 -Isrc

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

BIN = rfscan

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lhackrf -lsqlite3

clean:
	rm -f $(BIN) $(OBJ)
