scripts/restore_snapshot.sh#!/bin/bash
ARCHIVE=$1

if [ -z "$ARCHIVE" ]; then
    echo "Usage: ./restore_snapshot.sh <tar.gz file>"
    exit 1
fi

echo "[*] Restoring from $ARCHIVE..."
tar -xzf "$ARCHIVE"
echo "[+] Complete."
