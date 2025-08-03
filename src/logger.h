// logger.h
// version: v1.3.1

#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>

int logger_init(const char *csv_path, const char *sqlite_path);
void log_signal(const char *label, uint64_t freq, double rssi, int score, const char *uuid);
void logger_close(void);

#endif