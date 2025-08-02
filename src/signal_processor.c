// signal_processor.c
// Maps frequencies to labels + scores using protocols.def

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logger.h"

#define MAX_PROTOS 64

typedef struct {
    int start;
    int end;
    char label[32];
    int score;
} Protocol;

Protocol defs[MAX_PROTOS];
int def_count = 0;

void load_protocols(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[128];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || strlen(line) < 5) continue;
        int start, end, score;
        char label[32];
        if (sscanf(line, "%d %d %31s %d", &start, &end, label, &score) == 4) {
            defs[def_count].start = start;
            defs[def_count].end = end;
            strncpy(defs[def_count].label, label, sizeof(defs[def_count].label));
            defs[def_count].score = score;
            def_count++;
        }
    }

    fclose(f);
}

void process_signal(int freq, int rssi) {
    const char *label = "UNKNOWN";
    int score = 0;

    for (int i = 0; i < def_count; i++) {
        if (freq >= defs[i].start && freq <= defs[i].end) {
            label = defs[i].label;
            score = defs[i].score;
            break;
        }
    }

    log_signal(label, freq, rssi, score);
}
