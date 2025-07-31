// signal_map.c
// Tracks transient → persistent signals in memory
// Author: [You]

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "signal_map.h"

static signal_entry_t signals[MAX_SIGNALS];
static int signal_count = 0;

static uint64_t now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ((uint64_t)ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

void signal_map_init(void) {
    memset(signals, 0, sizeof(signals));
    signal_count = 0;
}

// Returns state:
// 1 = SeenOnce
// 2 = Confirmed
// 3 = Persistent
int signal_map_register(uint64_t freq, int rssi) {
    for (int i = 0; i < signal_count; i++) {
        if (signals[i].freq == freq) {
            signals[i].hits += 1;
            signals[i].last_seen = now_ms();
            signals[i].rssi = rssi;
            if (signals[i].hits >= 3) return 3;
            else return 2;
        }
    }

    if (signal_count < MAX_SIGNALS) {
        signals[signal_count].freq = freq;
        signals[signal_count].rssi = rssi;
        signals[signal_count].hits = 1;
        signals[signal_count].first_seen = now_ms();
        signals[signal_count].last_seen = signals[signal_count].first_seen;
        signal_count++;
        return 1;
    }

    return 0; // buffer full
}
