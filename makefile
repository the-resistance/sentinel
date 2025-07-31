# Makefile for rf-sentinel
CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lhackrf -lsqlite3

SRC = \
	src/main.c \
	src/sweep.c \
	src/band_filter.c \
	src/signal_map.c \
	src/logger.c

HDR = \
	src/sweep.h \
	src/band_filter.h \
	src/signal_map.h \
	src/logger.h \
	src/version.h

BIN = rfscan

all: $(BIN)

$(BIN): $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN) $(LDFLAGS)

clean:
	rm -f $(BIN)

.PHONY: all clean
