// main.c
// Sentinel spectrum monitor entry point
// version: v1.3.1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "logger.h"
#include "device.h"
#include "scanner.h"
#include "signal_map.h"
#include "uuid_utils.h"
#include "version.h"

int main(int argc, char *argv[]) {
    printf("%s\n", VERSION_STRING);

    config_t config = {0};
    if (parse_args(argc, argv, &config) != 0) {
        fprintf(stderr, "[-] Failed to parse arguments.\n");
        return 1;
    }

    if (logger_init("logs/signal_log.csv", "db/signal_log.db") != 0) {
        fprintf(stderr, "[-] Logger initialization failed.\n");
        return 1;
    }

    device_t dev = {0};
    if (init_device(&dev, config.device_index) != 0) {
        fprintf(stderr, "[-] Failed to initialize HackRF device.\n");
        logger_close();
        return 1;
    }

    for (uint64_t freq = config.start_freq; freq <= config.end_freq; freq += config.step_hz) {
        scan_frequency(&dev, freq, config.dwell_ms, &config);
    }

    logger_close();
    return 0;
}