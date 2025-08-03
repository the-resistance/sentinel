// scanner.h
// version: v1.3.1

#ifndef SCANNER_H
#define SCANNER_H

#include <stdint.h>
#include "device.h"
#include "signal_map.h"

void scan_frequency(device_t *dev, uint64_t freq, int dwell_ms, const config_t *config);

#endif