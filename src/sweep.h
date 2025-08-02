#ifndef SWEEP_H
#define SWEEP_H

#include <stdint.h>

// Sweep configuration
typedef struct {
    uint64_t start_freq_hz;
    uint64_t end_freq_hz;
    uint32_t step_hz;
    uint32_t dwell_ms;
} sweep_config_t;

void sweep_init(sweep_config_t *config);
uint64_t sweep_next_frequency();
int sweep_has_next();
void sweep_reset();

#endif // SWEEP_H