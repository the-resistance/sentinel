#!/bin/bash
# ─────────────────────────────────────────────
# Filename:    setup_sentinel.sh
# Version:     v1.0.1
# Author:      Kevin / System Architect
# Description: Installs dependencies, compiles scanner,
#              and prepares runtime environment.
# ─────────────────────────────────────────────

echo "[*] Updating package index..."
apt-get update -y

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
  findutils

echo "[*] Setting execution permissions..."
chmod +x run-monitor.sh scripts/*.sh db/*.sh

echo "[*] Creating output directories..."
mkdir -p logs captures bin db data scripts

echo "[*] Building Sentinel binary..."
make clean && make build

if [ $? -eq 0 ]; then
  echo "[✔] Build successful: bin/sentinel_v1.0.1.out"
else
  echo "[✘] Build failed."
  exit 1
fi
