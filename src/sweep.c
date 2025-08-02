#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <hackrf.h>
#include "sweep.h"
#include "logger.h"
#include "band_filter.h"

extern band_range_t ignore_bands[], tactical_bands[];
extern int ignore_count, tactical_count;

static hackrf_device* dev = NULL;

static int handle_callback(hackrf_transfer* transfer) {
    // Stubbed — you could write IQ or RSSI analysis here
    return 0;
}

int run_sweep(const sweep_config_t *cfg) {
    if (hackrf_init() != HACKRF_SUCCESS) {
        fprintf(stderr, "[!] hackrf_init() failed.\n");
        return -1;
    }

    if (hackrf_open(&dev) != HACKRF_SUCCESS) {
        fprintf(stderr, "[!] hackrf_open() failed.\n");
        return -1;
    }

    printf("[*] HackRF initialized\n");

    uint64_t freq = cfg->start_freq;
    while (freq <= cfg->end_freq) {
        if (in_any_band(freq, ignore_bands, ignore_count)) {
            freq += cfg->step_hz;
            continue;
        }

        int r = hackrf_set_freq(dev, freq);
        if (r != HACKRF_SUCCESS) {
            fprintf(stderr, "[!] Failed to tune: %" PRIu64 " Hz\n", freq);
            break;
        }

        hackrf_set_sample_rate(dev, cfg->sample_rate);
        hackrf_set_amp_enable(dev, 1);

        // Simulated hit every 10 MHz (for now)
        if ((freq / 1000000) % 10 == 0) {
            logger_record(freq, -50, "persistent");
            printf("[+] Signal at %" PRIu64 " Hz\n", freq);
        }

        usleep(cfg->dwell_ms * 1000);
        freq += cfg->step_hz;
    }

    hackrf_close(dev);
    hackrf_exit();
    return 0;
}
