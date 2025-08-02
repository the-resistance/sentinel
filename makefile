# makefile — Sentinel RF Scanner v0.9.2
# Author: Kevin / System Architect
# Description: Compiles the Sentinel RF scan suite with SQLite logging, vendor resolution, and band filters.

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -Isrc
LDFLAGS = -lm -lsqlite3
BIN_DIR = bin
SRC_DIR = src

OBJS = \
  $(SRC_DIR)/main.o \
  $(SRC_DIR)/logger.o \
  $(SRC_DIR)/device.o \
  $(SRC_DIR)/signal_processor.o \
  $(SRC_DIR)/band_filter.o

TARGET = $(BIN_DIR)/rfscan

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(SRC_DIR)/*.o
	rm -f $(TARGET)