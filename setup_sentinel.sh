#!/bin/bash
# setup_sentinel.sh — Full installer for Sentinel RF stack

echo "[*] Updating packages..."
apt-get update -y

echo "[*] Installing dependencies..."
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

echo "[*] Setting script permissions..."
chmod +x run-monitor.sh scripts/*.sh db/*.sh

echo "[*] Creating necessary directories..."
mkdir -p logs captures bin

echo "[*] Building project..."
make clean && make all

if [ $? -eq 0 ]; then
    echo "[✔] Build successful: bin/sentinel.out and bin/rfdash ready."
else
    echo "[✘] Build failed."
    exit 1
fi
