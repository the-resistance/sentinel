#!/usr/bin/env bash
# Run from an extracted package or checkout containing compiled binaries.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$ROOT/runtime"
OUT="${1:-$(mktemp -d "$ROOT/runtime/qualification-XXXXXX")}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
exec > >(tee "$OUT/suite.stdout") 2> >(tee "$OUT/suite.stderr" >&2)
"$ROOT/bin/sentinel" doctor --log-level trace --log-file "$OUT/doctor.jsonl"
"$ROOT/bin/test_core"
"$ROOT/bin/test_logging" 2>"$OUT/logger-stress.jsonl"
if [[ -x "$ROOT/bin/test_receiver" ]]; then "$ROOT/bin/test_receiver"; fi
bash "$ROOT/tests/integration.sh"
bash "$ROOT/tests/performance.sh" "$OUT/performance"
printf 'Qualification completed: %s\n' "$OUT"
