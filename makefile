CC ?= cc
CFLAGS ?= -Wall -Wextra -Werror -O2 -std=c11
BIN_DIR = bin
TARGET = $(BIN_DIR)/sentinel.out

.PHONY: all build clean dashboard
all: build
build: $(TARGET) $(BIN_DIR)/sentinel_v1.0.1.out
$(TARGET): src/main.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<
$(BIN_DIR)/sentinel_v1.0.1.out: $(TARGET)
	cp $(TARGET) $@
dashboard:
	@echo "Legacy dashboard has no unified data producer. Use Sentinel-RF/sentinel.py report."
clean:
	rm -f $(TARGET) $(BIN_DIR)/sentinel_v1.0.1.out

