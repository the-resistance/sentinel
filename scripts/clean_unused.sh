#!/bin/bash
# ─────────────────────────────────────────────
# Filename:    clean_unused.sh
# Version:     v1.0.1
# Author:      Kevin / System Architect
# Description: Cleans up files deprecated in v1.0.1.
#              Safe to run after version alignment.
# ─────────────────────────────────────────────

echo "[*] Removing deprecated files..."

# Known removed files from earlier development
rm -f scripts/analyze_heatmap.sh
rm -f scripts/restore_snapshot.sh
rm -f src/version.h

echo "[✔] Unused files removed."
