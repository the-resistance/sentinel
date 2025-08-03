// device.h
// version: v1.3.1

#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>

typedef struct {
    int device_index;
    uint64_t start_freq;
    uint64_t end_freq;
    uint64_t step_hz;
    int dwell_ms;
    int signal_threshold;
} config_t;

typedef struct {
    void *handle;
} device_t;

int parse_args(int argc, char **argv, config_t *config);
int init_device(device_t *dev, int index);
double measure_rssi(device_t *dev, uint64_t freq, int dwell_ms);

#endif