#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "band_filter.h"

int parse_band_line(char *line, band_range_t *out) {
    if (!line || !out) return -1;
    return sscanf(line, "%" SCNu64 " %" SCNu64, &out->start, &out->end) == 2 ? 0 : -1;
}

int load_bands(const char *filename, band_range_t *bands, int max) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return -1;

    char line[128];
    int count = 0;

    while (fgets(line, sizeof(line), fp) && count < max) {
        if (line[0] == '#' || strlen(line) < 3) continue;
        if (parse_band_line(line, &bands[count]) == 0) {
            count++;
        }
    }

    fclose(fp);
    return count;
}

int in_any_band(uint64_t freq, const band_range_t *bands, int count) {
    for (int i = 0; i < count; i++) {
        if (freq >= bands[i].start && freq <= bands[i].end) {
            return 1;
        }
    }
    return 0;
}
