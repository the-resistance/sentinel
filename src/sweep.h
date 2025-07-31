// sweep.h
// RF sweep engine interface for rf-sentinel
#ifndef SWEEP_H
#define SWEEP_H

#include <stdint.h>

typedef enum {
    MODE_FULL,
    MODE_TACTICAL
} sweep_mode_t;

typedef struct {
    uint64_t freq_start_hz;
    uint64_t freq_end_hz;
    uint64_t step_hz;
    int dwell_ms;
    int rssi_threshold;
    uint32_t sample_rate;
    sweep_mode_t mode;
} sweep_config_t;

int sweep_run(const sweep_config_t *cfg, volatile int *keep_running);

#endif
