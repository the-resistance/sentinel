// signal_processor.c — Scan Routines
// Version: 1.0.4

#define _XOPEN_SOURCE 700
#include <unistd.h>   // usleep()
#include <stdio.h>
#include <stdlib.h>
#include "device.h"
#include "logger.h"
#include "signal_processor.h"
#include "band_filter.h"
#include "config.h"

void run_quick_scan(device_t *dev) {
    for (float freq = 100.0; freq < 200.0; freq += 0.5) {
        int rssi = rand() % 100;
        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_event("QuickScan match", freq, rssi);
        }
        usleep(QUICKSCAN_DWELL_MS * 1000);
    }
}

void run_full_scan(device_t *dev) {
    for (float freq = 100.0; freq < 200.0; freq += 0.1) {
        int rssi = rand() % 100;
        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_event("FullScan match", freq, rssi);
        }
        usleep(FULLSCAN_DWELL_MS * 1000);
    }
}

void run_protocol_scan(device_t *dev) {
    for (float freq = 100.0; freq < 200.0; freq += 0.2) {
        int rssi = rand() % 100;
        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_event("Protocol match", freq, rssi);
        }
        usleep(PROTOCOLSCAN_DWELL_MS * 1000);
    }
}
