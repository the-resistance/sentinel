# Makefile — v1.1.0 — UUID support — 2025-08-02

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11

INCLUDES = -Isrc
SRC = src/main.c \
      src/logger.c \
      src/signal_processor.c \
      src/uuid_utils.c

OBJ = $(SRC:.c=.o)
BIN = bin/sentinel

all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f src/*.o
	rm -f $(BIN)