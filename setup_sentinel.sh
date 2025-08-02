#!/bin/bash
set -e

echo "[*] Updating system..."
sudo apt update

echo "[*] Installing Sentinel dependencies..."
sudo apt install -y \
  build-essential gcc \
  libncurses5-dev libncursesw5-dev \
  sqlite3 libsqlite3-dev \
  hackrf gnuradio gr-osmosdr \
  wireshark tshark aircrack-ng \
  git python3 python3-pip \
  python3-scapy net-tools jq dialog

echo "[*] Cloning Sentinel (if missing)..."
if [ ! -d "$HOME/sentinel" ]; then
    git clone https://github.com/the-resistance/sentinel.git "$HOME/sentinel"
else
    echo "[*] Repo already exists. Skipping clone."
fi

cd "$HOME/sentinel"

echo "[*] Creating required directories..."
mkdir -p bin db logs captures data scripts

echo "[*] Building project..."
make clean
make

echo "[*] Setting permissions..."
chmod +x run_monitor.sh scripts/*.sh

echo "[+] Setup complete. Use './run_monitor.sh' to start monitoring."
