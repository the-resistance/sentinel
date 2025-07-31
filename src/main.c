// main.c
// rf-sentinel v0.1.0
// Entry point for RF sweep tool using HackRF
// Author: [You]
// License: [Classified / Open as desired]

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "sweep.h"
#include "band_filter.h"
#include "signal_map.h"
#include "logger.h"
#include "version.h"

volatile int keep_running = 1;

void handle_sigint(int sig) {
    keep_running = 0;
    fprintf(stderr, "\n[!] Caught SIGINT, terminating sweep...\n");
}

void print_usage(const char *prog) {
    printf("Usage: %s --mode full|tactical --threshold -95 --dwell 100\n", prog);
    printf("Optional: --step 1000000 --rate 2000000\n");
}

int main(int argc, char *argv[]) {
    sweep_config_t cfg;

    // Defaults
    cfg.freq_start_hz = 1000000;
    cfg.freq_end_hz   = 6000000000;
    cfg.step_hz       = 1000000;
    cfg.dwell_ms      = 100;
    cfg.rssi_threshold = -95;
    cfg.sample_rate   = 2000000;
    cfg.mode          = MODE_FULL;

    printf("rf-sentinel v%s\n", VERSION_STRING);
    printf("https://github.com/your/repo\n\n");

    // Parse minimal args
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i+1 < argc) {
            if (strcmp(argv[i+1], "tactical") == 0) cfg.mode = MODE_TACTICAL;
            else cfg.mode = MODE_FULL;
            i++;
        } else if (strcmp(argv[i], "--threshold") == 0 && i+1 < argc) {
            cfg.rssi_threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--dwell") == 0 && i+1 < argc) {
            cfg.dwell_ms = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--step") == 0 && i+1 < argc) {
            cfg.step_hz = strtoul(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "--rate") == 0 && i+1 < argc) {
            cfg.sample_rate = strtoul(argv[++i], NULL, 10);
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    signal(SIGINT, handle_sigint);

    // Load ignore/tactical bands
    if (load_band_filters("data/ignore_bands.txt", "data/tactical_bands.txt") != 0) {
        fprintf(stderr, "[X] Failed to load band filters.\n");
        return 1;
    }

    // Init logger + hit map
    logger_init("logs/trace_index.csv", "db/signal_log.db");
    signal_map_init();

    // Begin sweep
    if (sweep_run(&cfg, &keep_running) != 0) {
        fprintf(stderr, "[X] Sweep failed or terminated abnormally.\n");
        return 1;
    }

    logger_close();
    printf("[✓] Sweep complete.\n");
    return 0;
}
