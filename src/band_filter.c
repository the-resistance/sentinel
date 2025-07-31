// band_filter.c
// Loads ignore and tactical band ranges from file
// Author: [You]

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "band_filter.h"

static band_range_t ignore_ranges[MAX_FILTERS];
static band_range_t tactical_ranges[MAX_FILTERS];
static int num_ignore = 0;
static int num_tactical = 0;

static int parse_range_line(const char *line, band_range_t *out) {
    if (strncmp(line, "range", 5) != 0) return -1;
    return sscanf(line + 6, "%llu %llu", &out->start, &out->end) == 2 ? 0 : -1;
}

int load_band_file(const char *filename, band_range_t *ranges, int *count) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || strlen(line) < 12) continue;
        band_range_t r;
        if (parse_range_line(line, &r) == 0) {
            if (*count < MAX_FILTERS) {
                ranges[*count] = r;
                (*count)++;
            }
        }
    }

    fclose(f);
    return 0;
}

int load_band_filters(const char *ignore_file, const char *tactical_file) {
    num_ignore = num_tactical = 0;
    int ok1 = load_band_file(ignore_file, ignore_ranges, &num_ignore);
    int ok2 = load_band_file(tactical_file, tactical_ranges, &num_tactical);
    return (ok1 == 0 && ok2 == 0) ? 0 : -1;
}

int band_should_ignore(uint64_t freq_hz) {
    for (int i = 0; i < num_ignore; i++) {
        if (freq_hz >= ignore_ranges[i].start && freq_hz <= ignore_ranges[i].end)
            return 1;
    }
    return 0;
}

int band_is_tactical(uint64_t freq_hz) {
    for (int i = 0; i < num_tactical; i++) {
        if (freq_hz >= tactical_ranges[i].start && freq_hz <= tactical_ranges[i].end)
            return 1;
    }
    return 0;
}
