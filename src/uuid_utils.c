// uuid_utils.c
// version: v1.3.1

#include <stdio.h>
#include <string.h>
#include "uuid_utils.h"

void generate_uuid(uint64_t freq, char *out, int out_size) {
    snprintf(out, out_size, "UUID-%llu", (unsigned long long)freq);
}