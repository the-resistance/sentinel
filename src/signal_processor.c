// signal_processor.c — Scan Routines
#define _POSIX_C_SOURCE 199309L

#include <time.h>     // nanosleep
#include <stdio.h>
#include <stdlib.h>
#include "device.h"
#include "signal_processor.h"
#include "logger.h"
#include "band_filter.h"
#include "config.h"

static void sleep_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

void run_quick_scan(device_t *dev) {
    (void)dev;
    for (int freq = 800; freq < 1000; freq += 5) {
        int rssi = rand() % 100;
        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_event("QuickScan match", freq, rssi);
        }
        sleep_ms(QUICKSCAN_DWELL_MS);
    }
}

void run_full_scan(device_t *dev) {
    (void)dev;
    for (int freq = 700; freq < 2600; freq += 1) {
        int rssi = rand() % 100;
        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_event("FullScan match", freq, rssi);
        }
        sleep_ms(FULLSCAN_DWELL_MS);
    }
}

void run_protocol_scan(device_t *dev) {
    (void)dev;
    for (int freq = 900; freq < 920; freq += 1) {
        int rssi = rand() % 100;
        if (rssi >= DEFAULT_RSSI_THRESHOLD) {
            log_event("ProtocolScan match", freq, rssi);
        }
        sleep_ms(PROTOCOLSCAN_DWELL_MS);
    }
}
