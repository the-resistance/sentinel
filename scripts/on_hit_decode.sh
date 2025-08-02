#!/bin/bash

FREQ=$1
TS=$(date +"%Y%m%d_%H%M%S")
OUTDIR="captures"
IQFILE="$OUTDIR/capture_${FREQ}_${TS}.iq"
PCAPFILE="$OUTDIR/decode_${FREQ}_${TS}.pcap"

mkdir -p "$OUTDIR"

echo "[*] Recording 10s IQ at $FREQ Hz..."
hackrf_transfer -f "$FREQ" -s 10000000 -n 10000000 -r "$IQFILE"

# Dispatch decoding based on band
if [[ "$FREQ" -ge 925000000 && "$FREQ" -le 1990000000 ]]; then
    echo "[*] Detected GSM band. Attempting decode..."
    timeout 30s grgsm_decode -r "$IQFILE" -o "$PCAPFILE"
elif [[ "$FREQ" -ge 2400000000 && "$FREQ" -le 2485000000 ]]; then
    echo "[*] Detected BLE/Wi-Fi band. Save IQ only for now."
else
    echo "[*] Band not recognized. Saved raw IQ for manual review."
fi
