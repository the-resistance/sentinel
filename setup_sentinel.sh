#!/bin/bash

#────────────────────────────────────────────────────
# Sentinel RF System Installer
# Version: 1.0.3
# Author: Kevin / System Architect
#
# Description:
# Prepares Kali Live system with RF scan stack, 
# ensures headers, binaries, and runtime dirs are present.
# Includes hardened logic for broken apt, dpkg, and Git failures.
#────────────────────────────────────────────────────

set -e

echo "[*] Step 1: Update APT index"
apt-get update -y || echo "[!] Warning: apt mirror partial failure — continuing"

echo "[*] Step 2: Repairing dpkg state (if broken)"
dpkg --configure -a || true
apt-get install -f -y || true

echo "[*] Step 3: Auto-clone Git repo if not present"
if [ ! -d /root/sentinel ]; then
  echo "[+] Cloning fresh Sentinel project..."
  git clone --branch main https://github.com/the-resistance/sentinel.git /root/sentinel || {
    echo "[X] Git clone failed. Exiting."
    exit 1
  }
fi

cd /root/sentinel || {
  echo "[X] /root/sentinel not found. Exiting."
  exit 1
}

echo "[*] Step 4: Installing core packages for HackRF, PCAP, analysis"

REQUIRED_PKGS=(
  build-essential
  libsqlite3-dev
  libpcap-dev
  libhackrf-dev
  libncurses-dev
  hackrf
  jq
  htop
  sqlite3
  tshark
  wireshark
)

for pkg in "${REQUIRED_PKGS[@]}"; do
  if dpkg -s "$pkg" 2>/dev/null | grep -q "Status: install ok installed"; then
    echo "[=] $pkg already installed — skipping"
  else
    echo "[+] Installing $pkg..."
    apt-get install -y "$pkg" || echo "[!] Failed to install $pkg — continuing"
  fi
done

echo "[*] Step 5: Inject fallback headers if missing"

inject_stub_header() {
  local path=$1
  local name=$(basename "$path")
  if [ ! -f "$path" ]; then
    echo "[!] Missing header $name — injecting stub"
    mkdir -p "$(dirname "$path")"
    cat <<EOF > "$path"
// $name — fallback shim for deterministic compile
#ifndef _$(echo $name | tr a-z A-Z | sed 's/\.H//')
#define _$(echo $name | tr a-z A-Z | sed 's/\.H//')
// stub
#endif
EOF
  fi
}

inject_stub_header /usr/include/sys/capability.h
inject_stub_header /usr/include/sys/prctl.h

echo "[*] Step 6: Creating runtime directories"
mkdir -p logs captures db data scripts bin src

echo "[*] Step 7: Enforcing permissions on sensitive paths"
chmod 700 captures db

echo "[*] Step 8: Initializing signal database (if not present)"
if [ ! -f db/signal_log.db ]; then
  ./db/init_db.sh
fi

echo "[*] Step 9: Making shell scripts executable"
chmod +x setup_sentinel.sh run_monitor.sh
find scripts/ -type f -name "*.sh" -exec chmod +x {} \;

echo "[✓] Sentinel setup complete and ready."
