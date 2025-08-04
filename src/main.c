\// main.c — Sentinel RF Scanner Entry Point
// Version: 1.0.3
// Author: Kevin / System Architect

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version.h"
#include "logger.h"
#include "device.h"
#include "signal_processor.h"
#include "band_filter.h"
#include "config.h"

int main(int argc, char *argv[]) {
    printf("───────────────────────────────────────────\n");
    printf(" %s\n", get_build_info());
    printf("───────────────────────────────────────────\n");

    if (argc < 3 || strcmp(argv[1], "--mode") != 0) {
        fprintf(stderr, "[!] Usage: %s --mode [quick|general|protocol]\n", argv[0]);
        return 1;
    }

    const char *mode = argv[2];
    device_t dev = {0};

    if (!init_device(&dev, 0)) {
        fprintf(stderr, "[!] HackRF device initialization failed.\n");
        return 1;
    }

    if (strcmp(mode, "protocol") == 0) {
        load_protocol_whitelist("data/protocol_defs.txt");
    } else {
        load_excluded_bands("data/bands_excluded.txt");
    }

    if (strcmp(mode, "quick") == 0) {
        printf("[*] Starting QuickScan...\n");
        run_quick_scan(&dev);
    } else if (strcmp(mode, "general") == 0) {
        printf("[*] Starting FullScan...\n");
        run_full_scan(&dev);
    } else if (strcmp(mode, "protocol") == 0) {
        printf("[*] Starting ProtocolScan...\n");
        run_protocol_scan(&dev);
    } else {
        fprintf(stderr, "[!] Unknown mode: %s\n", mode);
        shutdown_device(&dev);
        return 1;
    }

    shutdown_device(&dev);
    return 0;
}
