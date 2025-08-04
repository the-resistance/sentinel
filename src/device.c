// device.c — HackRF Interface
#include <stddef.h>  // NULL
#include <libhackrf/hackrf.h>
#include "device.h"

hackrf_device *device_handle = NULL;

int init_device(device_t *dev, int index) {
    (void)dev;  // unused
    (void)index;
    return 1;
}

void shutdown_device() {
    // Stub
}
