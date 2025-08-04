// main.c — Sentinel RF Scanner Entry Point
// Version: 1.2.0
// Author: Kevin / System Architect

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version.h"
#include "logger.h"
#include "device.h"
#include "signal_processor.h"
#include "band_filter.h"
#include "scanner.h"
#include "protocol_scan.h"
#include "config.h"

void print_usage(const char *exec) {
    fprintf(stderr, "[!] Usage: %s --mode [quick|general|protocol]\n", exec);
}

int main(int argc, char *argv[]) {
    printf("───────────────────────────────────────────\n");
    printf(" %s\n", get_build_info());
    printf("───────────────────────────────────────────\n");

    if (argc < 3 || strcmp(argv[1], "--mode") != 0) {
        print_usage(argv[0]);
        return 1;
    }

    const char *mode = argv[2];

    if (strcmp(mode, "quick") != 0 &&
        strcmp(mode, "general") != 0 &&
        strcmp(mode, "protocol") != 0) {
        print_usage(argv[0]);
        return 1;
    }

    // Device Init
    if (!init_device()) {
        fprintf(stderr, "[!] HackRF device initialization failed.\n");
        return 1;
    }

    // Load Band Exclusions
    load_excluded_bands("data/bands_excluded.txt");

    // Mode Dispatch
    if (strcmp(mode, "quick") == 0) {
        printf("[*] Starting QuickScan...\n");
        start_quick_scan(DWELL_QUICK_US);
    } else if (strcmp(mode, "general") == 0) {
        printf("[*] Starting GeneralScan...\n");
        start_general_scan(DWELL_GENERAL_US);
    } else if (strcmp(mode, "protocol") == 0) {
        printf("[*] Starting ProtocolScan...\n");
        start_protocol_scan("data/protocols.def", DWELL_PROTOCOL_US);
    }

    // Shutdown device
    shutdown_device();
    return 0;
}
