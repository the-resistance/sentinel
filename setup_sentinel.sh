#!/usr/bin/env bash
set -euo pipefail
BASE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
make -C "$BASE" build
exec "$BASE/bin/sentinel" doctor "$@"
