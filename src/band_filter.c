// band_filter.c — Sentinel Band Filtering
// Version: 1.0.3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static float excluded_bands[512][2];
static int excluded_count = 0;

static float protocol_whitelist[256];
static int protocol_count = 0;

void load_excluded_bands(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return;
    while (fscanf(fp, "%f %f", &excluded_bands[excluded_count][0], &excluded_bands[excluded_count][1]) == 2) {
        excluded_count++;
    }
    fclose(fp);
}

int is_band_excluded(float freq) {
    for (int i = 0; i < excluded_count; i++) {
        if (freq >= excluded_bands[i][0] && freq <= excluded_bands[i][1]) {
            return 1;
        }
    }
    return 0;
}

void load_protocol_whitelist(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return;
    float freq;
    while (fscanf(fp, "%f", &freq) == 1 && protocol_count < 256) {
        protocol_whitelist[protocol_count++] = freq;
    }
    fclose(fp);
}

int is_in_protocol_list(float freq) {
    for (int i = 0; i < protocol_count; ++i) {
        if (freq >= protocol_whitelist[i] - 0.1 && freq <= protocol_whitelist[i] + 0.1) {
            return 1;
        }
    }
    return 0;
}
