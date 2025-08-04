#!/bin/bash
# run-monitor.sh — Sentinel RF Monitoring Wrapper Script
# Version: 1.0.2
# Author: Kevin / System Architect
# Description:
#   Launches QuickScan → FullScan Loop
#   Verifies HackRF, initializes DB, logs hits, triggers decode/capture
#   Supports SCAN_PHASE env for rfdash integration

echo "[*] Sentinel Runtime Monitor — launch sequence initiated"

export SCAN_PHASE="BOOTSTRAP"

# Step 1: Verify HackRF connection
echo "[*] Verifying HackRF availability..."
if ! hackrf_info | grep -q "Found HackRF"; then
  echo "[!] HackRF not detected. Exiting."
  exit 1
fi

# Step 2: Ensure required directories
echo "[*] Ensuring runtime directories..."
mkdir -p logs captures db data scripts bin

# Step 3: Database initialization (if not already present)
if [ ! -f db/signal_log.db ]; then
  echo "[*] Initializing SQLite signal database..."
  chmod +x ./db/init_db.sh
  ./db/init_db.sh || { echo "[!] DB init failed. Exiting."; exit 1; }
fi

# Step 4: Kill conflicting processes on HackRF
echo "[*] Killing conflicting processes (rfkill equivalent)..."
fuser -k /dev/hackrf0 > /dev/null 2>&1

# Step 5: Launch QuickScan once
echo "[*] Running QuickScan pass..."
export SCAN_PHASE="QUICKSCAN"
./bin/sentinel.out --mode quick || { echo "[!] QuickScan failed."; exit 1; }

# Step 6: Begin infinite FullScan loop
while true; do
  echo "[*] Running FullScan pass..."
  export SCAN_PHASE="FULLSCAN"
  ./bin/sentinel.out --mode general

  echo "[*] Scan complete — sleeping 10 seconds."
  sleep 10
done
