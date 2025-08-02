#!/bin/bash
# Initialize SQLite schema manually

DB_PATH="db/signal_log.db"

mkdir -p db

echo "[*] Creating database at $DB_PATH"

sqlite3 "$DB_PATH" <<EOF
CREATE TABLE IF NOT EXISTS signals (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    freq INTEGER,
    rssi INTEGER,
    status TEXT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
EOF

echo "[✓] Done."
