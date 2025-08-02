// uuid_utils.c — v1.0.0 — UUID extraction logic — 2025-08-02

#include <stdio.h>
#include <string.h>
#include "uuid_utils.h"

void extract_uuid_from_payload(const unsigned char* data, size_t len, char* out_uuid, size_t out_size) {
    for (size_t i = 0; i + 16 <= len; ++i) {
        if (data[i+2] == 0xFB || data[i+2] == 0x6F) {
            snprintf(out_uuid, out_size,
                "%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X%02X%02X",
                data[i], data[i+1], data[i+2], data[i+3],
                data[i+4], data[i+5], data[i+6], data[i+7],
                data[i+8], data[i+9], data[i+10], data[i+11],
                data[i+12], data[i+13], data[i+14], data[i+15]
            );
            return;
        }
    }
    out_uuid[0] = '\0';
}