#!/bin/bash
# setup_sentinel.sh — Sentinel Environment Setup Script
# Version: 1.0.3
# Author: Kevin / System Architect
# Description:
#   Initializes /root/sentinel environment:
#   - Fixes dpkg, updates APT
#   - Installs RF/GSM dependencies
#   - Injects fallback headers
#   - Initializes directories, permissions, DB

set -e

echo "[*] Step 1: Update APT index"
apt-get update

echo "[*] Step 2: Repairing dpkg state (if broken)"
apt-get install -f -y || true
dpkg --configure -a || true

echo "[*] Step 3: Auto-clone Git repo if not present"
if [ ! -d /root/sentinel ]; then
  git clone https://github.com/the-resistance/sentinel /root/sentinel
fi

cd /root/sentinel

echo "[*] Step 4: Installing core packages for HackRF, PCAP, analysis"

apt-get install -y \
  build-essential \
  libsqlite3-dev \
  libpcap-dev \
  libhackrf-dev \
  libncurses-dev \
  hackrf \
  jq \
  htop \
  sqlite3 \
  tshark \
  wireshark || true

echo "[*] Step 5: Installing GSM tooling"
apt-get install -y \
  gr-gsm \
  libosmocore || true

echo "[*] Step 6: Inject fallback headers if missing"
mkdir -p /usr/include/linux/
if [ ! -f /usr/include/linux/prctl.h ]; then
  echo "[!] Missing header prctl.h — injecting stub"
  echo -e "#pragma once\n#define PR_SET_NAME 15\n#define PR_GET_NAME 16" > /usr/include/linux/prctl.h
fi

echo "[*] Step 7: Creating runtime directories"
mkdir -p logs captures db data scripts bin

echo "[*] Step 8: Enforcing permissions on sensitive paths"
find . -type f -iname "*.sh" -exec chmod +x {} \;
chmod +x run-monitor.sh

echo "[*] Step 9: Initializing signal database (if not present)"
if [ ! -f db/signal_log.db ]; then
  chmod +x db/init_db.sh
  ./db/init_db.sh || echo "[!] DB init failed — please check manually"
fi

echo "[✔] Sentinel environment setup complete."
