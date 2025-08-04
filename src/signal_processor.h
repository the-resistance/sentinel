// signal_processor.h — Scan API Header

#ifndef SIGNAL_PROCESSOR_H
#define SIGNAL_PROCESSOR_H

#include "device.h"

void run_quick_scan(device_t *dev);
void run_full_scan(device_t *dev);
void run_protocol_scan(device_t *dev);

#endif
