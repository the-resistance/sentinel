// protocol_scan.c — Protocol-Driven Band Scan
// Version: 1.1.0
// Author: Kevin / System Architect

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

void start_protocol_scan(const char *def_file, int dwell_us) {
    FILE *fp = fopen(def_file, "r");
    if (!fp) {
        fprintf(stderr, "[X] Failed to open protocol definition file: %s\n", def_file);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || strlen(line) < 5) continue;

        char name[64], start[32], end[32], priority[32], desc[128];
        if (sscanf(line, "%63[^|]|%31[^|]|%31[^|]|%31[^|]|%127[^\n]",
                   name, start, end, priority, desc) == 5) {

            float f_start = strtof(start, NULL);
            float f_end = strtof(end, NULL);

            printf("[*] Scanning %s: %.1f–%.1f MHz (%s)\n", name, f_start, f_end, priority);
            scan_range(f_start, f_end, dwell_us);
        }
    }

    fclose(fp);
}
