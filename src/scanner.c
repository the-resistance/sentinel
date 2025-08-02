#include "scanner.h"
#include <stdio.h>
#include <unistd.h>  // for usleep
#include <hackrf.h>

static hackrf_device* device = NULL;

void init_scanner() {
    if (hackrf_init() != HACKRF_SUCCESS) {
        fprintf(stderr, "[!] Failed to initialize HackRF library\n");
        return;
    }

    if (hackrf_open(&device) != HACKRF_SUCCESS) {
        fprintf(stderr, "[!] Failed to open HackRF device\n");
        return;
    }

    fprintf(stdout, "[+] HackRF initialized and ready\n");
}

void perform_scan(uint64_t start_freq, uint64_t end_freq, uint32_t step_hz) {
    if (device == NULL) {
        fprintf(stderr, "[!] HackRF not initialized\n");
        return;
    }

    for (uint64_t freq = start_freq; freq <= end_freq; freq += step_hz) {
        if (hackrf_set_freq(device, freq) != HACKRF_SUCCESS) {
            fprintf(stderr, "[!] Failed to tune to frequency: %llu\n", freq);
            continue;
        }

        fprintf(stdout, "[*] Tuning to %llu Hz...\n", freq);

        // Hold briefly to allow tuning to settle
        usleep(100000); // 100ms
    }
}

void shutdown_scanner() {
    if (device) {
        hackrf_close(device);
        device = NULL;
    }
    hackrf_exit();
    fprintf(stdout, "[*] HackRF shutdown complete\n");
}