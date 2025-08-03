// uuid_utils.h
// version: v1.3.1

#ifndef UUID_UTILS_H
#define UUID_UTILS_H

#include <stdint.h>

void generate_uuid(uint64_t freq, char *out, int out_size);

#endif