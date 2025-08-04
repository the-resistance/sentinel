#!/bin/bash
#
# init_db.sh — Initializes Sentinel SQLite DB
# Version: 1.0.0
# Author: Kevin / System Architect

DB_PATH="db/signal_log.db"

sqlite3 "$DB_PATH" <<EOF
CREATE TABLE IF NOT EXISTS signals (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    freq_mhz REAL NOT NULL,
    rssi_dbm REAL NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    protocol TEXT,
    decoded TEXT,
    anomaly INTEGER DEFAULT 0
);
EOF

echo "[+] signal_log.db initialized with signals schema."
