#ifndef SCANNER_H
#define SCANNER_H

#include <stdint.h>

void init_scanner();
void perform_scan(uint64_t start_freq, uint64_t end_freq, uint32_t step_hz);
void shutdown_scanner();

#endif // SCANNER_H