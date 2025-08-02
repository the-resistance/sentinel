# Makefile for Sentinel RF scanner

CC=gcc
CFLAGS=-Wall -O2
SRC_DIR=src
BIN_DIR=bin
INCLUDE=-I$(SRC_DIR)

SRCS=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(SRCS:.c=.o)

# Output binaries
RFSCAN=$(BIN_DIR)/rfscan
RFDASH=$(BIN_DIR)/rfdash

.PHONY: all clean build install

all: build

build: $(RFSCAN) $(RFDASH)

$(RFSCAN): $(SRC_DIR)/main.c $(SRC_DIR)/logger.c $(SRC_DIR)/signal_processor.c
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

$(RFDASH): rfdash.c
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ rfdash.c -lncurses

clean:
	rm -f $(BIN_DIR)/*
	rm -f $(SRC_DIR)/*.o

install:
	cp $(RFSCAN) /usr/local/bin/rfscan
	cp $(RFDASH) /usr/local/bin/rfdash
