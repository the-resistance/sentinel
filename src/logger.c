// logger.c
// Handles CSV + SQLite logging + real-time dashboard feed

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>

void log_to_csv(const char *label, int freq, int rssi, int score) {
    FILE *f = fopen("logs/scan_log.csv", "a");
    if (!f) return;

    time_t now = time(NULL);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(f, "%s,%d,%d,%s,%d\n", ts, freq, rssi, label, score);
    fclose(f);
}

void log_to_sqlite(const char *label, int freq, int rssi, int score) {
    sqlite3 *db;
    char *err = 0;

    int rc = sqlite3_open("db/signal_log.db", &db);
    if (rc) return;

    char *sql = "CREATE TABLE IF NOT EXISTS hits ("
                "timestamp TEXT, frequency INTEGER, rssi INTEGER, label TEXT, score INTEGER);";
    sqlite3_exec(db, sql, 0, 0, &err);

    time_t now = time(NULL);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));

    char stmt[512];
    snprintf(stmt, sizeof(stmt),
             "INSERT INTO hits (timestamp, frequency, rssi, label, score) "
             "VALUES ('%s', %d, %d, '%s', %d);",
             ts, freq, rssi, label, score);

    sqlite3_exec(db, stmt, 0, 0, &err);
    sqlite3_close(db);
}

void log_to_feed(int freq, int rssi) {
    FILE *f = fopen("logs/live_feed.log", "a");
    if (f) {
        fprintf(f, "%d %d\n", freq, rssi);
        fclose(f);
    }
}

void log_signal(const char *label, int freq, int rssi, int score) {
    log_to_csv(label, freq, rssi, score);
    log_to_sqlite(label, freq, rssi, score);
    log_to_feed(freq, rssi);
}
