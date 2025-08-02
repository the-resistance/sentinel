#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>

typedef struct {
    char label[64];
    uint64_t freq;
    int rssi;
    int score;
} DeviceInfo;

void identify_device(DeviceInfo *dev);
const char* resolve_vendor(uint64_t freq);

#endif