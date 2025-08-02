#!/bin/bash

# ───────────────────────────────────────
# Sentinel RF Monitoring Wrapper Script
# Launches: QuickScan → FullScan Loop
# Logs hits, triggers decode/capture
# ───────────────────────────────────────

echo "[*] Verifying HackRF connection..."
if ! hackrf_info | grep -q "Found HackRF"; then
  echo "[!] HackRF not detected. Exiting."
  exit 1
fi

echo "[*] Ensuring runtime directories..."
mkdir -p logs captures db data scripts

# Init database if missing
if [ ! -f db/signal_log.db ]; then
  echo "[*] Initializing SQLite signal database..."
  ./db/init_db.sh
fi

# Run QuickScan once (high-priority bands only)
echo "[*] Running initial QuickScan..."
./rfscan --mode quick

# Begin infinite full-spectrum sweep loop
while true; do
  echo "[*] Running ComprehensiveScan pass..."
  ./rfscan --mode general
  sleep 10
done
