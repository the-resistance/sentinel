#include "device.h"
#include <string.h>

void identify_device(DeviceInfo *dev) {
    // Placeholder logic for now
    if (dev->freq > 2400000000 && dev->freq < 2500000000) {
        strncpy(dev->label, "Possible 2.4GHz tracker", sizeof(dev->label));
        dev->score = 90;
    } else {
        strncpy(dev->label, "Unclassified", sizeof(dev->label));
        dev->score = 10;
    }
}

const char* resolve_vendor(uint64_t freq) {
    if (freq > 2400000000 && freq < 2500000000) return "BLE/Wi-Fi Range";
    if (freq > 902000000 && freq < 928000000) return "LoRa/ISM Band";
    return "Unknown Spectrum";
}