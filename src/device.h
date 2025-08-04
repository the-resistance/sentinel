// device.h — Device Interface Header

#ifndef DEVICE_H
#define DEVICE_H

#include <libhackrf/hackrf.h>

typedef struct {
    hackrf_device *handle;
} device_t;

int init_device(device_t *dev, int index);
void shutdown_device(device_t *dev);

#endif
