// device.c
// version: v1.3.1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "device.h"

int parse_args(int argc, char **argv, config_t *config) {
    if (argc < 2) return -1;
    config->start_freq = 100000000ULL;
    config->end_freq = 6000000000ULL;
    config->step_hz = 1000000;
    config->dwell_ms = 50;
    config->signal_threshold = 50;
    config->device_index = 0;
    return 0;
}

int init_device(device_t *dev, int index) {
    dev->handle = NULL;
    return 0;
}

double measure_rssi(device_t *dev, uint64_t freq, int dwell_ms) {
    (void)dev;
    (void)freq;
    (void)dwell_ms;
    return -45.0;
}