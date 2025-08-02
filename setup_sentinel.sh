#!/bin/bash

echo "[*] Updating package list..."
sudo apt update

echo "[*] Installing dependencies..."
sudo apt install -y \
    build-essential \
    libhackrf-dev \
    libsqlite3-dev \
    sqlite3 \
    git \
    cmake \
    hackrf \
    gnuradio

echo "[*] Cloning repository..."
TARGET_DIR="/home/kali/sentinel"

if [ -d "$TARGET_DIR" ]; then
    echo "[*] Removing existing $TARGET_DIR"
    rm -rf "$TARGET_DIR"
fi

git clone https://github.com/the-resistance/sentinel "$TARGET_DIR"

echo "[*] Setting script permissions..."
chmod +x "$TARGET_DIR"/db/init_db.sh

echo "[*] Building project..."
cd "$TARGET_DIR" || exit 1
make clean && make

echo "[✓] Setup complete."
echo "Run the scanner with:"
echo "  cd $TARGET_DIR"
echo "  ./rfscan --mode full --threshold -90 --dwell 100"
