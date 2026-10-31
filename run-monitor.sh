#!/usr/bin/env bash
set -euo pipefail
BASE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="${SENTINEL_RF_ROOT:-$BASE/../Sentinel-RF}"
exec "${PYTHON:-python3}" "$ROOT/sentinel.py" scan --sweeps 0 "$@"

