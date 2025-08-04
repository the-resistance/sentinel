#!/bin/bash

# setup_sentinel.sh — Environment bootstrap for Sentinel RF Scanner
# Version: 1.0.2
# Author: Kevin / System Architect

echo "[*] Step 1: Update APT index"
apt-get update

echo "[*] Step 2: Repairing dpkg state (if broken)"
apt-get install -f -y
dpkg --configure -a

echo "[*] Step 3: Auto-clone Git repo if not present"
if [ ! -d /root/sentinel ]; then
  git clone https://github.com/the-resistance/sentinel.git /root/sentinel
fi
cd /root/sentinel || exit 1

echo "[*] Step 4: Installing core packages for HackRF, PCAP, analysis"
PACKAGES=(
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

for pkg in "${PACKAGES[@]}"; do
  if dpkg -s "$pkg" &>/dev/null; then
    echo "[=] $pkg already installed — skipping"
  else
    echo "[+] Installing: $pkg"
    apt-get install -y "$pkg"
  fi
done

echo "[*] Step 5: Inject fallback headers if missing"
mkdir -p /usr/include/linux
if [ ! -f /usr/include/linux/prctl.h ]; then
  echo "[!] Missing header prctl.h — injecting stub"
  echo "// stub prctl.h" > /usr/include/linux/prctl.h
  echo "#define PR_SET_NAME 15" >> /usr/include/linux/prctl.h
  echo "#define PR_GET_NAME 16" >> /usr/include/linux/prctl.h
fi

echo "[*] Step 6: Creating runtime directories"
mkdir -p logs captures db data scripts bin

echo "[*] Step 7: Enforcing permissions on sensitive paths"
find . -type f -name "*.sh" -exec chmod +x {} \;
chmod +x run_monitor.sh
chmod +x db/init_db.sh

echo "[*] Step 8: Initializing signal database (if not present)"
if [ ! -f db/signal_log.db ]; then
  ./db/init_db.sh
fi

# ─────────────────────────────────────────────────────────────
# Step 9: Optional — Install GSM Signal Analysis Tools
# Description: Adds support for GSM sniffing, IMSI capture, Kalibrate offset testing
# ─────────────────────────────────────────────────────────────

if [[ "$1" == "--enable-gsm" ]]; then
  echo "[*] Step 9: Enabling GSM toolchain"

  echo "[=] Installing gr-gsm, libosmocore..."
  apt-get install -y gr-gsm libosmocore

  echo "[=] Cloning and building kalibrate-hackrf..."
  cd /opt || exit 1

  if [ ! -d kalibrate-hackrf ]; then
    git clone https://github.com/scateu/kalibrate-hackrf.git
  fi

  cd kalibrate-hackrf || exit 1
  apt-get install -y libtool libfftw3-dev autoconf automake

  ./bootstrap && ./configure && make && make install

  echo "[*] GSM stack installed."
  cd /root/sentinel || exit 1
else
  echo "[=] GSM toolchain not enabled (pass --enable-gsm to setup)"
fi

echo "[✔] Sentinel setup complete."
