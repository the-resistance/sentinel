#!/bin/bash
# ─────────────────────────────────────────────
# Filename:    run-monitor.sh
# Version:     v1.0.1
# Author:      Kevin / System Architect
# Description: Sentinel runtime orchestrator. Launches
#              initial QuickScan, enters FullScan loop,
#              logs hits, triggers decode phase.
# ─────────────────────────────────────────────

echo "[*] Verifying HackRF connection..."
if ! hackrf_info | grep -q "Found HackRF"; then
  echo "[!] HackRF not detected. Exiting."
  exit 1
fi

echo "[*] Preparing runtime environment..."
mkdir -p logs captures db

if [ ! -f db/signal_log.db ]; then
  echo "[*] Initializing SQLite signal database..."
  ./db/init_db.sh
fi

export SCAN_PHASE="QUICKSCAN"
echo "[*] Starting QuickScan..."
bin/sentinel_v1.0.1.out --mode quick > logs/live_feed.log

export SCAN_PHASE="FULLSCAN"
while true; do
  echo "[*] Executing FullScan pass..."
  bin/sentinel_v1.0.1.out --mode general >> logs/live_feed.log

  if [ -x scripts/on_hit_decode.sh ]; then
    echo "[*] Triggering protocol decode..."
    scripts/on_hit_decode.sh
  fi

  sleep 10
done
