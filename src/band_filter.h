#ifndef BAND_FILTER_H
#define BAND_FILTER_H

#include <stdint.h>

#define MAX_BANDS 128

typedef struct {
    uint64_t start;
    uint64_t end;
} band_range_t;

int load_bands(const char *filename, band_range_t *bands, int max);
int in_any_band(uint64_t freq, const band_range_t *bands, int count);

#endif
