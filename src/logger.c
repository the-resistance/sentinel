#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "logger.h"

#define LOG_FILE "logs/signal_log.csv"

void init_logger(const char* filepath) {
    FILE* fp = fopen(filepath, "w");
    if (fp) {
        fprintf(fp, "timestamp,label,freq,rssi,score,uuid\n");
        fclose(fp);
    }
}

void log_signal(const char* label, uint64_t freq, int rssi, int score, const char* uuid) {
    FILE* fp = fopen(LOG_FILE, "a");
    if (!fp) return;

    time_t now = time(NULL);
    struct tm* tinfo = localtime(&now);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tinfo);

    fprintf(fp, "%s,%s,%lu,%d,%d,%s\n",
        timebuf,
        label,
        freq,
        rssi,
        score,
        uuid ? uuid : "null"
    );

    fclose(fp);
}