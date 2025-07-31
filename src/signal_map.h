// signal_map.h
// Runtime signal state tracking (in RAM)
#ifndef SIGNAL_MAP_H
#define SIGNAL_MAP_H

#include <stdint.h>

#define MAX_SIGNALS 2048

typedef struct {
    uint64_t freq;
    int rssi;
    int hits;
    uint64_t first_seen;
    uint64_t last_seen;
} signal_entry_t;

void signal_map_init(void);
int signal_map_register(uint64_t freq, int rssi);

#endif
