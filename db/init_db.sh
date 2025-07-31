#!/bin/bash
# init_db.sh
# Creates the SQLite database for rf-sentinel logging

DB_PATH="db/signal_log.db"

echo "[*] Creating SQLite database at $DB_PATH..."
mkdir -p db

sqlite3 "$DB_PATH" <<EOF
CREATE TABLE IF NOT EXISTS signals (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    freq INTEGER NOT NULL,
    rssi INTEGER NOT NULL,
    status TEXT NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
EOF

if [ $? -eq 0 ]; then
    echo "[✓] Database initialized."
else
    echo "[X] Failed to create database."
fi
