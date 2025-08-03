// signal_map.c
// version: v1.3.1

#include "signal_map.h"

const char* resolve_signal(uint64_t freq) {
    if (freq >= 2400000000ULL && freq <= 2485000000ULL) return "Bluetooth / WiFi";
    if (freq >= 902000000ULL && freq <= 928000000ULL) return "ISM 900MHz";
    if (freq >= 1575000000ULL && freq <= 1580000000ULL) return "GPS L1";
    return "Unknown";
}