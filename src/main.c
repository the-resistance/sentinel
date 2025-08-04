// main.c — Sentinel RF Scanner Entry Point
// Version: 1.0.1
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

    if (argc < 2) {
        fprintf(stderr, "[!] Usage: %s --mode [quick|general]\n", argv[0]);
        return 1;
    }

    const char *mode = argv[2] ? argv[2] : "";

    if (strcmp(argv[1], "--mode") != 0 || 
        (strcmp(mode, "quick") != 0 && strcmp(mode, "general") != 0)) {
        fprintf(stderr, "[!] Invalid arguments. Expected: --mode [quick|general]\n");
        return 1;
    }

    // Phase: Device Initialization
    if (!init_device()) {
        fprintf(stderr, "[!] HackRF device initialization failed.\n");
        return 1;
    }

    // Phase: Band Filtering Setup
    load_excluded_bands("data/bands_excluded.txt");

    // Phase: Scanning Execution
    if (strcmp(mode, "quick") == 0) {
        printf("[*] Starting QuickScan...\n");
        run_quick_scan();
    } else if (strcmp(mode, "general") == 0) {
        printf("[*] Starting FullScan...\n");
        run_full_scan();
    }

    // Phase: Cleanup
    shutdown_device();

    return 0;
}
