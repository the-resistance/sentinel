#!/bin/bash
# setup_sentinel.sh — v1.4.0 — Full protocol & DB support — 2025-08-02

echo "[+] Updating packages..."
sudo apt update && sudo apt install -y \
  build-essential gcc make git \
  libpcap-dev libhackrf-dev hackrf \
  gnuradio gqrx-sdr gr-osmosdr \
  aircrack-ng bluez bluetooth rfcat \
  gpsd gpsd-clients inspectrum \
  net-tools network-manager \
  wireshark tshark sqlite3 libsqlite3-dev \
  python3 python3-pip python3-scapy

echo "[+] Installing protocol tools via pip..."
pip3 install --break-system-packages \
  bleah killerbee scapy-radio

echo "[+] Cloning or updating repo..."
REPO_DIR="/home/kali/sentinel"

if [ ! -d "$REPO_DIR" ]; then
  git clone https://github.com/the-resistance/sentinel.git "$REPO_DIR"
else
  cd "$REPO_DIR" && git pull
fi

echo "[+] Creating runtime directories..."
mkdir -p "$REPO_DIR/logs"
mkdir -p "$REPO_DIR/bin"
mkdir -p "$REPO_DIR/db"

echo "[+] Fixing script & binary permissions..."
find "$REPO_DIR" -type f -name "*.sh" -exec chmod +x {} \;
find "$REPO_DIR" -type f -name "*.py" -exec chmod +x {} \;
chmod +x "$REPO_DIR/bin/sentinel"

echo "[+] Building project..."
cd "$REPO_DIR"
make clean && make

echo "[+] Initializing SQLite signal_log.db..."
sqlite3 "$REPO_DIR/db/signal_log.db" <<EOF
CREATE TABLE IF NOT EXISTS signals (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp TEXT,
    label TEXT,
    freq INTEGER,
    rssi INTEGER,
    score INTEGER,
    uuid TEXT
);
EOF

echo "[✓] Setup complete. Run: $REPO_DIR/bin/sentinel"