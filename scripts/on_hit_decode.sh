#!/bin/bash
# ─────────────────────────────────────────────
# Filename:    on_hit_decode.sh
# Version:     v1.0.1
# Author:      Kevin / System Architect
# Description: Optional post-signal trigger for deeper
#              decode. Can run PCAP analysis, pattern
#              recognition, or custom protocol match.
# ─────────────────────────────────────────────

DECODE_LOG="logs/decode.log"

echo "[*] on_hit_decode.sh started at $(date)" >> "$DECODE_LOG"

# Example: Process most recent PCAP (if capture exists)
LATEST_PCAP=$(ls -t captures/*.pcap 2>/dev/null | head -n 1)

if [[ -f "$LATEST_PCAP" ]]; then
  echo "[*] Decoding $LATEST_PCAP..." >> "$DECODE_LOG"
  # Insert custom decode logic here, e.g.:
  # tshark -r "$LATEST_PCAP" >> "$DECODE_LOG"
  echo "[+] Protocol decode complete." >> "$DECODE_LOG"
else
  echo "[!] No PCAP file found to decode." >> "$DECODE_LOG"
fi
