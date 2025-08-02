#!/bin/bash
DB="db/signal_log.db"

echo "[*] Top 10 persistent frequencies:"
sqlite3 "$DB" "
SELECT frequency, COUNT(*) AS hits
FROM hits
GROUP BY frequency
ORDER BY hits DESC
LIMIT 10;
"
