// sweep.c
// HackRF RF sweep loop for rf-sentinel
// Author: [You]
// License: [Classified / Open as desired]

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <hackrf.h>
#include "sweep.h"
#include "band_filter.h"
#include "signal_map.h"
#include "logger.h"

#define MAX_BUFFER 262144

static hackrf_device *device = NULL;

static int measure_signal(int *rssi_out) {
    int8_t buffer[MAX_BUFFER];
    int sum = 0;

    int status = hackrf_start_rx(device, NULL, NULL);
    if (status != HACKRF_SUCCESS) return -1;

    usleep(1000); // brief wait

    status = hackrf_set_amp_enable(device, 1);
    if (status != HACKRF_SUCCESS) return -2;

    status = hackrf_set_lna_gain(device, 40);
    status |= hackrf_set_vga_gain(device, 20);
    if (status != HACKRF_SUCCESS) return -3;

    status = hackrf_start_rx(device, NULL, NULL);
    if (status != HACKRF_SUCCESS) return -4;

    status = hackrf_is_streaming(device);
    if (status != HACKRF_TRUE) return -5;

    // simulate signal read
    for (int i = 0; i < MAX_BUFFER; i++) buffer[i] = rand() % 255 - 128;

    for (int i = 0; i < MAX_BUFFER; i++) sum += abs(buffer[i]);
    int avg = sum / MAX_BUFFER;

    *rssi_out = -avg;  // crude proxy
    return 0;
}

int sweep_run(const sweep_config_t *cfg, volatile int *keep_running) {
    uint64_t freq = cfg->freq_start_hz;
    uint64_t end = cfg->freq_end_hz;

    if (hackrf_init() != HACKRF_SUCCESS) {
        fprintf(stderr, "[X] hackrf_init failed\n");
        return -1;
    }

    if (hackrf_open(&device) != HACKRF_SUCCESS) {
        fprintf(stderr, "[X] Failed to open HackRF device\n");
        return -2;
    }

    printf("[*] Sweep initialized: %llu Hz → %llu Hz in %llu Hz steps\n",
        cfg->freq_start_hz, cfg->freq_end_hz, cfg->step_hz);

    while (freq <= end && *keep_running) {
        if (band_should_ignore(freq)) {
            freq += cfg->step_hz;
            continue;
        }

        if (hackrf_set_freq(device, freq) != HACKRF_SUCCESS) {
            fprintf(stderr, "[!] Frequency set failed at %llu Hz\n", freq);
            freq += cfg->step_hz;
            continue;
        }

        usleep(cfg->dwell_ms * 1000);

        int rssi = 0;
        if (measure_signal(&rssi) != 0) {
            fprintf(stderr, "[!] Signal measure failed\n");
            freq += cfg->step_hz;
            continue;
        }

        if (rssi >= cfg->rssi_threshold) {
            int status = signal_map_register(freq, rssi);
            if (status >= 2) {
                logger_record(freq, rssi, status);
            }
        }

        freq += cfg->step_hz;
    }

    hackrf_close(device);
    hackrf_exit();
    return 0;
}
