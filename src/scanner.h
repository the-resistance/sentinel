// scanner.h — Frequency Scanner Interface
// Version: 1.1.0

#ifndef SCANNER_H
#define SCANNER_H

void start_quick_scan(int dwell_us);
void start_general_scan(int dwell_us);
void scan_range(float start, float end, int dwell_us);
void scan_single_frequency(float freq_mhz, int dwell_us);
int is_band_excluded(float freq_mhz);

#endif
