// device.c — Device Interface
#include <stddef.h>
#include <libhackrf/hackrf.h>
#include "device.h"

hackrf_device *device_handle = NULL;

int init_device(device_t *dev, int index) {
    (void)index; // Unused
    return 1;
}
