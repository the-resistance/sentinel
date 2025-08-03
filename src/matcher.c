// matcher.c
// Version: 1.0.0
// Purpose: Match scanned frequency against known protocol ranges

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEF_FILE "protocols.def"
#define LINE_MAX 256

typedef struct {
    char protocol[32];
    float freq;
    char units[8];
} ProtocolDef;

int match_protocol(float freq_mhz, const char* def_path) {
    FILE* f = fopen(def_path, "r");
    if (!f) {
        fprintf(stderr, "Error: Cannot open %s\n", def_path);
        return 1;
    }

    char line[LINE_MAX];
    while (fgets(line, LINE_MAX, f)) {
        if (line[0] == '#' || strlen(line) < 5) continue;

        ProtocolDef def;
        if (sscanf(line, "%31s %f %7s", def.protocol, &def.freq, def.units) == 3) {
            float target = def.freq;
            if (strstr(def.units, "GHz")) target *= 1000.0;
            if (strstr(def.units, "KHz")) target /= 1000.0;

            if ((freq_mhz > target - 1.0) && (freq_mhz < target + 1.0)) {
                printf("Match: %s (%.3f MHz)\n", def.protocol, freq_mhz);
                fclose(f);
                return 0;
            }
        }
    }

    printf("No match: %.3f MHz\n", freq_mhz);
    fclose(f);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <freq_MHz>\n", argv[0]);
        return 1;
    }

    float freq = atof(argv[1]);
    return match_protocol(freq, DEF_FILE);
}