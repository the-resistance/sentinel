#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "logger.h"
#include "uuid_utils.h"

void process_signal(const char* label, uint64_t freq, int rssi, int score, const unsigned char* payload, size_t length) {
    char uuid_buf[128] = {0};

    // Try to extract UUID (BLE, Apple, etc.)
    extract_uuid_from_payload(payload, length, uuid_buf, sizeof(uuid_buf));

    log_signal(label, freq, rssi, score, uuid_buf[0] ? uuid_buf : "null");
}