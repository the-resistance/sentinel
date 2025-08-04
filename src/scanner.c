// scanner.c — Frequency Sweep + Signal Logging
// Version: 1.1.0
// Author: Kevin / System Architect

#include <stdio.h>
#include <unistd.h>
#include "scanner.h"
#include "signal_processor.h"
#include "logger.h"
#include "config.h"

void start_quick_scan(int dwell_us) {
    printf("[*] QuickScan: Scanning high-priority bands...\n");
    scan_range(902.0, 928.0, dwell_us);     // ISM band
    scan_range(2402.0, 2480.0, dwell_us);   // BLE/Zigbee
}

void start_general_scan(int dwell_us) {
    printf("[*] General Scan: Full-range sweep excluding known noise...\n");

    float start_freq = 30.0;
    float end_freq = 3000.0;

    for (float f = start_freq; f <= end_freq; f += STEP_SIZE_MHZ) {
        if (!is_band_excluded(f)) {
            scan_single_frequency(f, dwell_us);
        }
    }
}

void scan_range(float start, float end, int dwell_us) {
    for (float f = start; f <= end; f += STEP_SIZE_MHZ) {
        scan_single_frequency(f, dwell_us);
    }
}

void scan_single_frequency(float freq_mhz, int dwell_us) {
    float signal_dbm = read_rssi(freq_mhz);

    if (signal_dbm >= RSSI_MIN_DBM && signal_dbm <= RSSI_MAX_DBM) {
        log_signal_hit(freq_mhz, signal_dbm);
    }

    usleep(dwell_us);
}
