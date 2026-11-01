#!/usr/bin/env bash
# Synthetic CPU/SQLite/replay qualification; does not open an RF receiver.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$ROOT/runtime"
OUT="${1:-$(mktemp -d "$ROOT/runtime/performance-XXXXXX")}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
TIME_BIN="${SENTINEL_TIME_BIN:-/usr/bin/time}"
[[ -x "$TIME_BIN" ]] || { echo 'Install the time package for process-level measurements.' >&2; exit 1; }
"$ROOT/bin/test_core" --fixture "$OUT/fixtures"
"$ROOT/bin/sentinel_bench" --iterations "${SENTINEL_BENCH_ITERATIONS:-20}" >"$OUT/benchmark.jsonl" 2>"$OUT/benchmark.stderr"
awk -F, 'NR==1 {for(n=0;n<1000;n++){printf "%s,12:%02d:%02d",$1,int(n/60),n%60;for(i=3;i<=NF;i++)printf ",%s",$i;printf "\n"}}' "$OUT/fixtures/sweep.csv" >"$OUT/replay-1000.csv"
for level in error trace; do
 "$TIME_BIN" -f '{"type":"process_benchmark","name":"replay_1000_sweeps","log_level":"'"$level"'","wall_seconds":%e,"user_seconds":%U,"system_seconds":%S,"max_rss_kib":%M,"exit":%x}' -o "$OUT/replay-$level-timing.json" \
  "$ROOT/bin/sentinel" run --replay "$OUT/replay-1000.csv" --log-level "$level" --workdir "$OUT/replay-$level" >"$OUT/replay-$level.stdout" 2>"$OUT/replay-$level.stderr"
 "$ROOT/bin/sentinel" report --log-level error --workdir "$OUT/replay-$level" >"$OUT/report-$level.csv"
 grep -q ',1000,-25.000000000' "$OUT/report-$level.csv"
done
cmp "$OUT/report-error.csv" "$OUT/report-trace.csv"
cat "$OUT/benchmark.jsonl" "$OUT/replay-error-timing.json" "$OUT/replay-trace-timing.json"
printf 'Results and high-verbosity diagnostics: %s\n' "$OUT" >&2
