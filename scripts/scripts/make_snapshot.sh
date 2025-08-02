#!/bin/bash
TS=$(date +"%Y%m%d_%H%M%S")
ARCHIVE="sentinel_snapshot_$TS.tar.gz"

echo "[*] Creating snapshot archive: $ARCHIVE"
tar -czf "$ARCHIVE" \
    rfscan *.c *.h *.sh Makefile README.md \
    data/ db/ scripts/ captures/ logs/ run_monitor.sh

echo "[+] Done."
