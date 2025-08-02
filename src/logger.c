#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>
#include <inttypes.h>
#include "logger.h"

static FILE *csv = NULL;
static sqlite3 *db = NULL;

void logger_init(const char *csv_path, const char *sqlite_path) {
    csv = fopen(csv_path, "a");
    if (!csv) {
        fprintf(stderr, "[!] Failed to open CSV log: %s\n", csv_path);
    }

    if (sqlite3_open(sqlite_path, &db) != SQLITE_OK) {
        fprintf(stderr, "[!] SQLite open failed: %s\n", sqlite3_errmsg(db));
        db = NULL;
        return;
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS signals ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "freq INTEGER,"
                      "rssi INTEGER,"
                      "status TEXT,"
                      "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";

    char *errmsg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        fprintf(stderr, "[!] SQLite schema error: %s\n", errmsg);
        sqlite3_free(errmsg);
    }
}

void logger_record(uint64_t freq_hz, int rssi, const char *status_str) {
    time_t now = time(NULL);
    char *ts = ctime(&now);
    ts[strlen(ts) - 1] = '\0';  // trim newline

    if (csv) {
        fprintf(csv, "%s,%" PRIu64 ",%d,%s\n", ts, freq_hz, rssi, status_str);
        fflush(csv);
    }

    if (db) {
        char sql[256];
        snprintf(sql, sizeof(sql),
            "INSERT INTO signals (freq, rssi, status) VALUES (%" PRIu64 ", %d, '%s');",
            freq_hz, rssi, status_str);
        sqlite3_exec(db, sql, NULL, NULL, NULL);
    }
}

void logger_close(void) {
    if (csv) fclose(csv);
    if (db) sqlite3_close(db);
}
