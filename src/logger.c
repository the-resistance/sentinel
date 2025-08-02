#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "logger.h"

void logger_record(FILE *csv, const char *ts, uint64_t freq_hz, int rssi, const char *status_str) {
    if (!csv || !ts || !status_str) return;
    fprintf(csv, "%s,%" PRIu64 ",%d,%s\n", ts, freq_hz, rssi, status_str);
    fflush(csv);
}
