// main.c — Sentinel RF Scanner Entry Point
// Version: 1.0.2
// Author: Kevin / System Architect

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version.h"
#include "logger.h"
#include "device.h"
#include "signal_processor.h"
#include "band_filter.h"

int main(int argc, char *argv[]) {
    printf("───────────────────────────────────────────\n");
    printf(" %s\n", get_build_info());
    printf("───────────────────────────────────────────\n");

    if (argc < 3 || strcmp(argv[1], "--mode") != 0) {
        fprintf(stderr, "[!] Usage: %s --mode [quick|general]\n", argv[0]);
        return 1;
    }

    const char *mode = argv[2];

    if (strcmp(mode, "quick") != 0 && strcmp(mode, "general") != 0) {
        fprintf(stderr, "[!] Invalid scan mode: %s\n", mode);
        return 1;
    }

    device_t dev = {0};  // Initialize device struct

    // Phase: Device Initialization
    if (!init_device(&dev, 0)) {
        fprintf(stderr, "[!] HackRF device initialization failed.\n");
        return 1;
    }

    // Phase: Band Filtering Setup
    load_excluded_bands("data/bands_excluded.txt");
    load_protocol_watchlist("data/protocol_watchlist.def");

    // Phase: Scanning Execution
    if (strcmp(mode, "quick") == 0) {
        printf("[*] Starting QuickScan...\n");
        run_quick_scan(&dev);
    } else {
        printf("[*] Starting FullScan...\n");
        run_full_scan(&dev);
    }

    // Phase: Cleanup
    shutdown_device(&dev);

    return 0;
}
