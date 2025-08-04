# ─────────────────────────────────────────────
# Filename:    makefile
# Version:     v1.0.1
# Author:      Kevin / System Architect
# Description: Compiles Sentinel RF scanner suite
#              with SQLite3 logging and HackRF backend.
# ─────────────────────────────────────────────

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -Isrc
LDFLAGS = -lm -lsqlite3 -lhackrf
BIN_DIR = bin
SRC_DIR = src

OBJS = \
  $(SRC_DIR)/main.o \
  $(SRC_DIR)/logger.o \
  $(SRC_DIR)/device.o \
  $(SRC_DIR)/signal_processor.o \
  $(SRC_DIR)/band_filter.o \
  $(SRC_DIR)/scanner.o \
  $(SRC_DIR)/matcher.o \
  $(SRC_DIR)/uuid_utils.o \
  $(SRC_DIR)/sweep.o \
  $(SRC_DIR)/signal_map.o

TARGET = $(BIN_DIR)/sentinel_v1.0.1.out

.PHONY: all clean build dashboard

all: build

build: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

dashboard: $(BIN_DIR)/rfdash

$(BIN_DIR)/rfdash: rfdash.c
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ rfdash.c -lncurses

clean:
	rm -f $(SRC_DIR)/*.o
	rm -f $(TARGET)
	rm -f $(BIN_DIR)/rfdash
