// scanner.c
// version: v1.3.1

#include <stdio.h>
#include <unistd.h>
#include "scanner.h"
#include "signal_map.h"
#include "signal_processor.h"
#include "logger.h"

void scan_frequency(device_t *dev, uint64_t freq, int dwell_ms, const config_t *config) {
    double rssi = measure_rssi(dev, freq, dwell_ms);
    int score = analyze_rssi(freq, rssi);

    if (score > config->signal_threshold) {
        const char *label = resolve_signal(freq);
        char uuid[64] = {0};
        generate_uuid(freq, uuid, sizeof(uuid));
        log_signal(label, freq, rssi, score, uuid);
    }
}