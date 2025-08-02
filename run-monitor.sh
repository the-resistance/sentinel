#!/bin/bash

echo "[*] Checking for HackRF..."
if ! hackrf_info | grep -q "Found HackRF"; then
    echo "[!] HackRF not detected. Aborting."
    exit 1
fi

echo "[*] Ensuring data directories exist..."
mkdir -p db logs data

# Optional: initialize DB manually
if [ ! -f db/signal_log.db ]; then
    echo "[*] Initializing SQLite database..."
    ./db/init_db.sh
fi

# Optional: create default band filters if missing
for f in data/ignore_bands.txt data/tactical_bands.txt; do
    if [ ! -f "$f" ]; then
        echo "[*] Creating placeholder: $f"
        echo "# <start_freq> <end_freq>" > "$f"
    fi
done

echo "[*] Launching rfscan..."
./rfscan

echo "[✓] Scan finished."
