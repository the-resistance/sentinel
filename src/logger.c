// logger.c
// version: v1.3.1

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "logger.h"

static FILE *csv_fp = NULL;
static sqlite3 *db = NULL;

int logger_init(const char *csv_path, const char *sqlite_path) {
    csv_fp = fopen(csv_path, "a");
    if (!csv_fp) return -1;

    if (sqlite3_open(sqlite_path, &db) != SQLITE_OK) {
        fclose(csv_fp);
        return -1;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS signals ("
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "label TEXT, freq INTEGER, rssi REAL, score INTEGER, uuid TEXT);";
    return sqlite3_exec(db, sql, 0, 0, 0) == SQLITE_OK ? 0 : -1;
}

void log_signal(const char *label, uint64_t freq, double rssi, int score, const char *uuid) {
    if (csv_fp) fprintf(csv_fp, "%s,%llu,%.2f,%d,%s\n", label, freq, rssi, score, uuid);
    if (db) {
        char sql[512];
        snprintf(sql, sizeof(sql),
            "INSERT INTO signals (label, freq, rssi, score, uuid) "
            "VALUES ('%s', %llu, %.2f, %d, '%s');",
            label, freq, rssi, score, uuid);
        sqlite3_exec(db, sql, 0, 0, 0);
    }
}

void logger_close(void) {
    if (csv_fp) fclose(csv_fp);
    if (db) sqlite3_close(db);
}