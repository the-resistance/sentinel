// band_filter.h
// Frequency suppression and tactical range filters
#ifndef BAND_FILTER_H
#define BAND_FILTER_H

#include <stdint.h>

#define MAX_FILTERS 64

typedef struct {
    uint64_t start;
    uint64_t end;
} band_range_t;

int load_band_filters(const char *ignore_file, const char *tactical_file);
int band_should_ignore(uint64_t freq_hz);
int band_is_tactical(uint64_t freq_hz);

#endif
