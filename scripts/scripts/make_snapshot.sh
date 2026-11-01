#!/usr/bin/env bash
set -euo pipefail
BASE="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT="${1:-$PWD/sentinel_snapshot_$(date +%Y%m%d_%H%M%S).tar.gz}"
tar -C "$BASE" -czf "$OUT" src data db scripts makefile README.md MANIFEST.md run-monitor.sh setup_sentinel.sh
echo "$OUT"

