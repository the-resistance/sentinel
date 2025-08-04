// device.c — HackRF Device Interface
// Version: 1.0.3

#include "device.h"
#include <libhackrf/hackrf.h>

hackrf_device *device_handle = NULL;

int init_device(device_t *dev, int index) {
    if (hackrf_init() != HACKRF_SUCCESS) return 0;
    if (hackrf_open(&device_handle) != HACKRF_SUCCESS) return 0;
    dev->handle = device_handle;
    return 1;
}

void shutdown_device(device_t *dev) {
    if (dev && dev->handle) {
        hackrf_close(dev->handle);
        hackrf_exit();
        dev->handle = NULL;
    }
}
