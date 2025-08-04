// logger.c — Event Logger
// Version: 1.0.3

#include <stdio.h>
#include "logger.h"

void log_event(const char *event, float freq, int rssi) {
    printf("[LOG] %s | Freq: %.2f MHz | RSSI: %d\n", event, freq, rssi);
}
