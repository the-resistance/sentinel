// band_filter.h — Sentinel Band Filter Header

#ifndef BAND_FILTER_H
#define BAND_FILTER_H

void load_excluded_bands(const char *filename);
int is_band_excluded(float freq);

void load_protocol_whitelist(const char *filename);
int is_in_protocol_list(float freq);

#endif
