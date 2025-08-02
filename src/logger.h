#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdio.h>

void logger_init(const char *csv_path, const char *sqlite_path);
void logger_record(uint64_t freq_hz, int rssi, const char *status_str);
void logger_close(void);

#endif
