#!/bin/bash

# Sentinel Setup Script — v1.0.3
# System-wide install and hardening for RF scanner
# Author: Kevin / the-resistance
# Target Dir: /root/sentinel

echo "[*] Sentinel Setup Initiated"

# ───────────────────────────────────────────────
# Step 1: Update APT index
# ───────────────────────────────────────────────
echo "[*] Step 1: Update APT index"
apt-get update -y

# ───────────────────────────────────────────────
# Step 2: Repair broken dpkg state (if needed)
# ───────────────────────────────────────────────
echo "[*] Step 2: Repairing dpkg state (if broken)"
apt-get install -f -y
dpkg --configure -a

# ───────────────────────────────────────────────
# Step 3: Auto-clone Git repo if not present
# ───────────────────────────────────────────────
if [ ! -d "/root/sentinel" ]; then
  echo "[*] Cloning Sentinel repository..."
  git clone https://github.com/the-resistance/sentinel.git /root/sentinel
fi
cd /root/sentinel || exit 1

# ───────────────────────────────────────────────
# Step 4: Core dependencies (HackRF, analysis)
# ───────────────────────────────────────────────
REQUIRED_PACKAGES=(
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
  ntpdate
)

echo "[*] Step 4: Installing core packages for HackRF, PCAP, analysis"
for pkg in "${REQUIRED_PACKAGES[@]}"; do
  if dpkg -s "$pkg" >/dev/null 2>&1; then
    echo "[=] $pkg already installed — skipping"
  else
    echo "[+] Installing $pkg..."
    apt-get install -y "$pkg"
  fi
done

# ───────────────────────────────────────────────
# Step 5: Inject fallback headers if missing
# ───────────────────────────────────────────────
if ! grep -q "PR_SET_NAME" /usr/include/linux/prctl.h 2>/dev/null; then
  echo "[!] Missing header prctl.h — injecting stub"
  mkdir -p /usr/include/linux
  echo "#define PR_SET_NAME 15" > /usr/include/linux/prctl.h
fi

# ───────────────────────────────────────────────
# Step 6: Create required runtime directories
# ───────────────────────────────────────────────
echo "[*] Step 6: Creating runtime directories"
mkdir -p logs captures db data scripts bin

# ───────────────────────────────────────────────
# Step 7: Enforce permissions
# ───────────────────────────────────────────────
echo "[*] Step 7: Enforcing permissions on sensitive paths"
find . -type f -name "*.sh" -exec chmod +x {} \;
chmod +x run-monitor.sh

# ───────────────────────────────────────────────
# Step 8: Signal DB init
# ───────────────────────────────────────────────
echo "[*] Step 8: Initializing signal database (if not present)"
if [ -f "./db/init_db.sh" ]; then
  chmod +x ./db/init_db.sh
  ./db/init_db.sh
else
  echo "[!] Missing init_db.sh"
fi

# ───────────────────────────────────────────────
# Step 9: Set Timezone + NTP Sync
# ───────────────────────────────────────────────
echo "[*] Setting timezone to Pacific (America/Los_Angeles)..."
ln -sf /usr/share/zoneinfo/America/Los_Angeles /etc/localtime
dpkg-reconfigure -f noninteractive tzdata

echo "[*] Syncing system clock with NTP pool..."
ntpdate pool.ntp.org

# ───────────────────────────────────────────────
# Step 10: Optional GSM Tools from Source
# ───────────────────────────────────────────────
echo "[*] Installing optional GSM tools (gr-gsm, kalibrate-hackrf)..."

# gr-gsm
if [ ! -d "/opt/gr-gsm" ]; then
  git clone https://github.com/ptrkrysik/gr-gsm /opt/gr-gsm
  cd /opt/gr-gsm
  mkdir build && cd build
  cmake ..
  make -j$(nproc)
  make install
  ldconfig
  ln -s /usr/local/bin/grgsm_livemon /usr/bin/grgsm_livemon 2>/dev/null
  echo "[+] gr-gsm installed"
fi

# kalibrate-hackrf
if [ ! -d "/opt/kalibrate-hackrf" ]; then
  git clone https://github.com/scateu/kalibrate-hackrf /opt/kalibrate-hackrf
  cd /opt/kalibrate-hackrf
  ./bootstrap
  ./configure
  make -j$(nproc)
  make install
  ln -s /usr/local/bin/kal /usr/bin/kal 2>/dev/null
  echo "[+] kalibrate-hackrf installed"
fi

# ───────────────────────────────────────────────
# Final Status
# ───────────────────────────────────────────────
echo "[✓] Sentinel environment initialized"
echo "[i] Run scanner via: ./run-monitor.sh"
