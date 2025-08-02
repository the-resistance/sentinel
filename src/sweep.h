#ifndef SWEEP_H
#define SWEEP_H

#include <stdint.h>

typedef struct {
    uint64_t start_freq;
    uint64_t end_freq;
    uint64_t step_hz;
    uint32_t sample_rate;
    uint32_t dwell_ms;
} sweep_config_t;

int run_sweep(const sweep_config_t *cfg);

#endif
