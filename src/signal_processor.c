// signal_processor.c — Scan Routines
// Version: 1.0.4

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>     // rand()
#include <unistd.h>     // usleep()
#include "device.h"
#include "signal_processor.h"
#include "logger.h"
#include "band_filter.h"
#include "config.h"

void run_quick_scan(device_t *dev) {
    for (int freq = 900000000; freq < 915000000; freq += 100000) {
        int rssi = rand() % 100;

        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_event("QuickScan match", freq, rssi);
        }

        usleep(QUICKSCAN_DWELL_MS * 1000);
    }
}

void run_full_scan(device_t *dev) {
    for (int freq = 900000000; freq < 915000000; freq += 50000) {
        int rssi = rand() % 100;

        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_event("FullScan hit", freq, rssi);
        }

        usleep(FULLSCAN_DWELL_MS * 1000);
    }
}

void run_protocol_scan(device_t *dev) {
    int freqs[] = {935000000, 940000000, 950000000, 960000000};
    for (int i = 0; i < 4; ++i) {
        int rssi = rand() % 100;

        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_event("ProtocolScan hit", freqs[i], rssi);
        }

        usleep(PROTOCOLSCAN_DWELL_MS * 1000);
    }
}
