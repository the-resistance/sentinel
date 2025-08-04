// logger.c — Signal Hit Logging
// Version: 1.0.1

#include "logger.h"
#include <stdio.h>
#include <sqlite3.h>
#include <time.h>

void log_signal_hit(float freq, int rssi) {
    FILE *fp = fopen("logs/live_feed.log", "a");
    if (!fp) return;
    time_t t = time(NULL);
    fprintf(fp, "[%s] Freq: %.2f MHz | RSSI: %d\n", ctime(&t), freq, rssi);
    fclose(fp);
}
