// signal_processor.c — Scan Routines
// Version: 1.0.3

#include "device.h"
#include "logger.h"
#include "band_filter.h"
#include "config.h"
#include <unistd.h>
#include <stdio.h>

void run_quick_scan(device_t *dev) {
    for (float f = 70.0; f <= 6000.0; f += 2.5) {
        if (is_band_excluded(f)) continue;
        int rssi = rand() % 100;
        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_signal_hit(f, rssi);
        }
        usleep(QUICKSCAN_DWELL_MS * 1000);
    }
}

void run_full_scan(device_t *dev) {
    for (float f = 30.0; f <= 6000.0; f += 0.5) {
        if (is_band_excluded(f)) continue;
        int rssi = rand() % 100;
        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_signal_hit(f, rssi);
        }
        usleep(FULLSCAN_DWELL_MS * 1000);
    }
}

void run_protocol_scan(device_t *dev) {
    for (float f = 30.0; f <= 6000.0; f += 0.25) {
        if (!is_in_protocol_list(f)) continue;
        int rssi = rand() % 100;
        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_signal_hit(f, rssi);
        }
        usleep(PROTOCOLSCAN_DWELL_MS * 1000);
    }
}
