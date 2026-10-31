#!/usr/bin/env bash
# Validate the supported runtime; dependency installation is explicit.
set -euo pipefail
BASE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="${SENTINEL_RF_ROOT:-$BASE/../Sentinel-RF}"
[[ -f "$ROOT/sentinel.py" ]] || { echo "Set SENTINEL_RF_ROOT to your Sentinel-RF checkout." >&2; exit 2; }
echo "Required: Python 3.10+, numpy, scipy, hackrf tools; build-essential for the C launcher."
exec "${PYTHON:-python3}" "$ROOT/sentinel.py" doctor "$@"

