// logger.h — v1.1.0 — UUID logging added — 2025-08-02

#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>

void init_logger(const char* filepath);
void log_signal(const char* label, uint64_t freq, int rssi, int score, const char* uuid);

#endif