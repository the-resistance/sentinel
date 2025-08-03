// signal_processor.c
// version: v1.3.1

#include "signal_processor.h"

int analyze_rssi(uint64_t freq, double rssi) {
    if (rssi > -30.0) return 100;
    if (rssi > -50.0) return 75;
    if (rssi > -70.0) return 50;
    return 0;
}