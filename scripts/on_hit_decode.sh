#!/bin/bash

FREQ=$1
TS=$(date +"%Y%m%d_%H%M%S")
OUTDIR="captures"
IQFILE="$OUTDIR/capture_${FREQ}_${TS}.iq"
PCAPFILE="$OUTDIR/decode_${FREQ}_${TS}.pcap"

mkdir -p "$OUTDIR"

echo "[*] Recording 10s at $FREQ Hz..."
hackrf_transfer -f "$FREQ" -s 10000000 -n 10000000 -r "$IQFILE"

# Dispatch decoder based on frequency
if [[ "$FREQ" -ge 925000000 && "$FREQ" -le 1990000000 ]]; then
    echo "[*] Running GSM decoder (gr-gsm)..."
    timeout 30s grgsm_decode -r "$IQFILE" -o "$PCAPFILE"
elif [[ "$FREQ" -ge 2400000000 && "$FREQ" -le 2483500000 ]]; then
    echo "[*] 2.4 GHz detected (Wi-Fi/BLE). No direct PCAP decode yet."
    # You may build and insert gr-bluetooth or ieee802-11 logic here
else
    echo "[*] No decoder assigned for this band. IQ file saved."
fi
