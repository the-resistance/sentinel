// uuid_utils.h — v1.0.0 — UUID extraction helper — 2025-08-02

#ifndef UUID_UTILS_H
#define UUID_UTILS_H

#include <stddef.h>

void extract_uuid_from_payload(const unsigned char* data, size_t len, char* out_uuid, size_t out_size);

#endif