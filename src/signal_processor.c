// signal_processor.c — Scan Routines
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // usleep
#include "device.h"
#include "signal_processor.h"
#include "logger.h"
#include "band_filter.h"
#include "config.h"

void run_quick_scan(device_t *dev) {
    (void)dev;
    for (int freq = 900000000; freq < 915000000; freq += 200000) {
        int rssi = rand() % 100;
        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_event("QuickScan match", freq, rssi);
        }
        usleep(QUICKSCAN_DWELL_MS * 1000);
    }
}

void run_full_scan(device_t *dev) {
    (void)dev;
    for (int freq = 800000000; freq < 2000000000; freq += 500000) {
        int rssi = rand() % 100;
        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_event("FullScan match", freq, rssi);
        }
        usleep(FULLSCAN_DWELL_MS * 1000);
    }
}

void run_protocol_scan(device_t *dev) {
    (void)dev;
    for (int freq = 850000000; freq < 890000000; freq += 100000) {
        int rssi = rand() % 100;
        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_event("ProtoScan match", freq, rssi);
        }
        usleep(PROTOCOLSCAN_DWELL_MS * 1000);
    }
}
