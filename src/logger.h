#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>

// Initialize logger (optional, not yet used)
void init_logger(const char* filepath);

// Core log function used by signal processor
void log_signal(const char* label, uint64_t freq, int rssi, int score);

#endif