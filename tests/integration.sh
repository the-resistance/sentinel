#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
REAL="$ROOT/bin/sentinel"
TEST="$ROOT/bin/test_sentinel"
TEMP="$(mktemp -d)"
CHILD=""
checks=0
cleanup() {
 if [[ -n "$CHILD" ]]; then kill -TERM "$CHILD" 2>/dev/null || true; wait "$CHILD" 2>/dev/null || true; fi
 rm -rf "$TEMP"
}
trap cleanup EXIT
expect() {
 local expected="$1"; shift
 local result=0
 "$@" >"$TEMP/stdout" 2>"$TEMP/stderr" || result=$?
 if [[ "$result" != "$expected" ]]; then cat "$TEMP/stderr" >&2; echo "expected $expected, got $result: $*" >&2; exit 1; fi
 checks=$((checks+1))
}
"$ROOT/bin/test_core" --fixture "$TEMP/fixtures"
expect 0 env PATH=/nonexistent "$REAL" run --replay "$TEMP/fixtures/sweep.csv" --captures-dir "$TEMP/fixtures" --workdir "$TEMP/results with spaces"
grep -q '"state":"complete"' "$TEMP/results with spaces"/runs/*/run.json
grep -q '1325000.000000000,10000.000000000' "$TEMP/results with spaces"/runs/*/spectrum_report.csv
grep -q ',2,-25.000000000' "$TEMP/results with spaces"/runs/*/spectrum_report.csv
grep -q '"samples_analyzed":16384' "$TEMP/results with spaces"/runs/*/analysis.jsonl
expect 0 "$REAL" report --workdir "$TEMP/results with spaces"
expect 0 "$REAL" analyze --input "$TEMP/fixtures/tone.cs8" --workdir "$TEMP/analysis"
cat "$TEMP/fixtures/sweep.csv" "$TEMP/fixtures/sweep.csv" >"$TEMP/backwards.csv"
expect 1 "$REAL" run --replay "$TEMP/backwards.csv" --workdir "$TEMP/backwards"
awk '{print;print}' "$TEMP/fixtures/sweep.csv" >"$TEMP/duplicates.csv"
expect 0 "$REAL" run --replay "$TEMP/duplicates.csv" --workdir "$TEMP/duplicates"
grep -q ',2,-25.000000000' "$TEMP/duplicates"/runs/*/spectrum_report.csv
expect 2 "$REAL" run --replay "$TEMP/fixtures/sweep.csv" --capture
expect 2 "$REAL" run --lna 3
expect 2 "$REAL" run --margin-db nan
expect 2 "$REAL" run --sweeps 0
expect 2 "$REAL" run --duration 1000000
expect 1 "$REAL" report --workdir "$TEMP/missing"
expect 1 "$REAL" run --replay "$TEMP/absent.csv" --workdir "$TEMP/absent"
: >"$TEMP/empty.csv"
expect 1 "$REAL" run --replay "$TEMP/empty.csv" --workdir "$TEMP/empty"
cp "$TEMP/fixtures/sweep.csv" "$TEMP/bad.csv"; printf 'bad\n' >>"$TEMP/bad.csv"
expect 2 "$REAL" run --replay "$TEMP/bad.csv" --workdir "$TEMP/partial"
grep -q '"state":"partial"' "$TEMP/partial"/runs/*/run.json
expect 1 env SN_TEST_OPEN_FAIL=1 "$TEST" run --workdir "$TEMP/open-failure"
expect 1 env SN_TEST_READ_FAIL=1 "$TEST" scan --workdir "$TEMP/read-failure" --sweeps 1
expect 1 env SN_TEST_CLOSE_FAIL=1 "$TEST" scan --workdir "$TEMP/close-failure" --start-mhz 902 --stop-mhz 918 --sweeps 1
expect 1 env SN_TEST_STALL=1 "$TEST" scan --workdir "$TEMP/timeout" --timeout .02 --sweeps 1
expect 0 "$TEST" run --capture --duration .01 --sample-rate 2000000 --start-mhz 902 --stop-mhz 918 --sweeps 1 --workdir "$TEMP/live"
grep -Eq '"captures_analyzed":[1-9]' "$TEMP/live"/runs/*/run.json
expect 2 env SN_TEST_CAPTURE_FAIL=1 "$TEST" run --capture --duration .01 --sample-rate 2000000 --start-mhz 902 --stop-mhz 918 --sweeps 1 --workdir "$TEMP/partial-capture"
grep -q '"state":"partial"' "$TEMP/partial-capture"/runs/*/run.json
expect 0 "$TEST" capture --freq-mhz 915 --duration .01 --sample-rate 2000000 --workdir "$TEMP/capture"
printf 'odd' >"$TEMP/fixtures/bad.cs8"
expect 2 "$REAL" analyze --captures-dir "$TEMP/fixtures" --workdir "$TEMP/mixed-captures"
cp "$TEMP/fixtures/tone.cs8" "$TEMP/fixtures/mismatch.cs8"
printf 'freq_mhz=915\nsample_rate=2000000\nsamples=1\nstatus=complete\n' >"$TEMP/fixtures/mismatch.meta"
expect 1 "$REAL" analyze --input "$TEMP/fixtures/mismatch.cs8" --workdir "$TEMP/mismatched"
printf 'freq_mhz=915\nsample_rate=2000000\nstatus=complete_extra\n' >"$TEMP/fixtures/mismatch.meta"
expect 1 "$REAL" analyze --input "$TEMP/fixtures/mismatch.cs8" --workdir "$TEMP/bad-status"
printf 'freq_mhz=915\nsample_rate=2000000\nsample_format=cf32\n' >"$TEMP/fixtures/mismatch.meta"
expect 1 "$REAL" analyze --input "$TEMP/fixtures/mismatch.cs8" --workdir "$TEMP/bad-format"
printf 'freq_mhz=915\nsample_rate=2000000\nsample_rate=2000000\n' >"$TEMP/fixtures/mismatch.meta"
expect 1 "$REAL" analyze --input "$TEMP/fixtures/mismatch.cs8" --workdir "$TEMP/duplicate-meta"
mkfifo "$TEMP/pipe.csv" "$TEMP/pipe.cs8"
expect 1 "$REAL" run --replay "$TEMP/pipe.csv" --workdir "$TEMP/pipe-replay"
expect 1 "$REAL" analyze --input "$TEMP/pipe.cs8" --sample-rate 2000000 --freq-mhz 915 --workdir "$TEMP/pipe-iq"
mkdir -p "$TEMP/corrupt"; printf 'not a database' >"$TEMP/corrupt/sentinel.sqlite"
expect 1 "$REAL" run --replay "$TEMP/fixtures/sweep.csv" --workdir "$TEMP/corrupt"
env SN_TEST_STALL=1 "$TEST" scan --workdir "$TEMP/interrupted" --sweeps 0 >"$TEMP/bg.out" 2>"$TEMP/bg.err" &
CHILD=$!
for ((i=0;i<200;i++)); do if compgen -G "$TEMP/interrupted/runs/*/events.jsonl" >/dev/null; then break; fi; sleep .01; done
expect 1 "$TEST" scan --workdir "$TEMP/locked" --sweeps 1
kill -TERM "$CHILD"; result=0; wait "$CHILD" || result=$?; CHILD=""
[[ "$result" == 130 ]]
grep -q '"state":"interrupted"' "$TEMP/interrupted"/runs/*/run.json
expect 0 "$TEST" scan --workdir "$TEMP/after-interrupt" --start-mhz 902 --stop-mhz 918 --sweeps 1
printf 'native integration: %s CLI checks plus report/cleanup assertions passed\n' "$checks"
