#!/bin/bash
# setup_sentinel.sh v1.2.0
# Fully automated setup for the Sentinel RF Sweep System (Kali Linux)

set -e

SENTINEL_DIR="$HOME/sentinel"
LOGFILE="$HOME/logs/setup.log"
REPO_URL="https://github.com/the-resistance/sentinel.git"

echo "[+] Logging setup to $LOGFILE"
mkdir -p "$HOME/logs"
exec > >(tee -a "$LOGFILE") 2>&1

echo "[+] Installing required packages..."
sudo apt update
sudo apt install -y \
  build-essential libhackrf-dev libsqlite3-dev \
  sqlite3 hackrf git make gcc

echo "[+] Cloning or updating repository..."
if [[ -d "$SENTINEL_DIR/.git" ]]; then
  cd "$SENTINEL_DIR"
  git pull
else
  git clone "$REPO_URL" "$SENTINEL_DIR"
  cd "$SENTINEL_DIR"
fi

echo "[+] Creating runtime directories..."
mkdir -p "$SENTINEL_DIR/bin"
mkdir -p "$SENTINEL_DIR/logs"
mkdir -p "$SENTINEL_DIR/db"
mkdir -p "$SENTINEL_DIR/captures"
mkdir -p "$SENTINEL_DIR/data"

echo "[+] Fixing permissions..."
chmod +x "$SENTINEL_DIR"/*.sh
chmod +x "$SENTINEL_DIR/scripts/"*.sh || true

echo "[+] Building project..."
make -C "$SENTINEL_DIR" clean
make -C "$SENTINEL_DIR"

if [[ ! -f "$SENTINEL_DIR/bin/sentinel" ]]; then
  echo "[!] Build failed. Sentinel binary not found."
  exit 1
fi

echo "[+] Initializing SQLite DB..."
if [[ ! -f "$SENTINEL_DIR/db/signal_log.db" ]]; then
  sqlite3 "$SENTINEL_DIR/db/signal_log.db" "VACUUM;"
  echo "[+] signal_log.db created."
else
  echo "[i] signal_log.db already exists."
fi

echo "[+] Setup complete."
echo "[i] Run monitor with: $SENTINEL_DIR/run-monitor.sh"
echo "[i] Binary: $SENTINEL_DIR/bin/sentinel"