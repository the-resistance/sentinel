#!/bin/bash
# ─────────────────────────────────────────────
# Filename:    setup_sentinel.sh
# Version:     v1.0.1
# Author:      Kevin / System Architect
# Description: Installs dependencies, waits for full
#              system update to complete, then builds
#              Sentinel and applies permissions.
# ─────────────────────────────────────────────

set -e  # Exit immediately on error

echo "[*] Updating system package index (blocking)..."
apt-get update -y && echo "[✔] apt-get update completed."

echo "[*] Installing required packages..."
apt-get install -y \
  build-essential \
  libhackrf-dev \
  hackrf \
  hackrf-tools \
  libpcap-dev \
  tcpdump \
  sqlite3 \
  libsqlite3-dev \
  ncurses-bin \
  libncurses5-dev \
  coreutils \
  jq \
  htop \
  binutils \
  file \
  findutils && echo "[✔] Package install completed."

echo "[*] Ensuring script permissions are set..."
chmod +x run-monitor.sh scripts/*.sh db/*.sh

echo "[*] Creating runtime directories if missing..."
mkdir -p logs captures bin db data scripts

echo "[*] Building Sentinel binary..."
make clean && make build

if [ $? -eq 0 ]; then
  echo "[✔] Build complete: bin/sentinel_v1.0.1.out"
else
  echo "[✘] Build failed. Check output."
  exit 1
fi
