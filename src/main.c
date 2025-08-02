#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "version.h"
#include "band_filter.h"
#include "logger.h"
#include "sweep.h"

band_range_t ignore_bands[MAX_BANDS], tactical_bands[MAX_BANDS];
int ignore_count = 0, tactical_count = 0;

int main() {
    ignore_count = load_bands("data/ignore_bands.txt", ignore_bands, MAX_BANDS);
    tactical_count = load_bands("data/tactical_bands.txt", tactical_bands, MAX_BANDS);

    printf("[*] Loaded %d ignore bands, %d tactical bands\n", ignore_count, tactical_count);

    logger_init("logs/trace_index.csv", "db/signal_log.db");

    sweep_config_t cfg = {
        .start_freq = 1000000,
        .end_freq   = 6000000000ULL,
        .step_hz    = 1000000,
        .sample_rate = 10000000,
        .dwell_ms = 75
    };

    run_sweep(&cfg);
    logger_close();
    return 0;
}
