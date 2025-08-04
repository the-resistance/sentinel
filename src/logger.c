#include <stdio.h>
#include "logger.h"

void log_event(const char *label, int frequency, int rssi) {
    printf("[LOG] %s — Freq: %d Hz, RSSI: %d\n", label, frequency, rssi);
}
