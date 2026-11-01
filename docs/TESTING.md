# Native qualification and performance

Target: Linux ELF, built natively for the compiler host architecture. This is not a Windows PE executable. The GitHub Actions package targets Ubuntu 24.04 x86_64 and dynamically links SQLite/libhackrf; compatible runtime libraries are required (`libsqlite3-0 libhackrf0` on Debian/Ubuntu). For Kali or another architecture, build locally using `make`. No Python runtime is required.

## Compile and run

Install `build-essential libsqlite3-dev libhackrf-dev time`, then:

```sh
make test
make benchmark
bash tests/run-suite.sh
make sanitize
make package
```

`make package` generates `dist/sentinel-linux-ARCH.tar.gz`. Extract into an empty directory and run `bash tests/run-suite.sh`; prebuilt production, benchmark and test executables are included. `dist/BUILD.txt` records the source commit (when built in a git checkout), compiler and host; verify binary integrity with `sha256sum -c dist/SHA256SUMS`. `bin/test_sentinel` is a deliberately simulated receiver executable; `bin/sentinel` is the production executable. Do not substitute the test executable for field measurements. The suite never opens a physical receiver. Reports are kept in `runtime/qualification-XXXXXX` unless an output directory is supplied. Output locations should be new/private directories. `SENTINEL_BENCH_ITERATIONS=100 bash tests/performance.sh` increases benchmark work (1..10000).

The correctness suite checks CSV/calendar geometry, 2000 deterministic malformed inputs, known tones/two tones/silence, recurrence persistence, receiver lifecycle and late callbacks through a controlled API shim, partial outputs, missing/corrupt files, locking, timeout/interruption cleanup, and diagnostic behavior. Logger tests exercise escaping, severity filtering, long-message truncation, 8 MiB rotation, errno/error preservation, nonregular/symlink rejection and a real file-size-limit write failure. CLI tests verify trace records, explicit files, invalid levels, failed file opens and quiet logging.

The performance suite produces JSONL for parsing 64-bin rows, detection over 4096 bins, waveform description of 65536 IQ pairs and in-memory SQLite recurrence. Each benchmark validates its output, runs seven batches and reports batch p50/p95, total time and throughput. With seven observations the nearest-rank p95 is the slowest batch; it is a small-sample diagnostic, not a statistically stable tail estimate. These are batch durations, not per-operation latency. Compiler, system, architecture, SQLite version, peak RSS and CPU usage accompany results. Sanitized runs are for correctness, not comparable production throughput.

A separate end-to-end replay runs 1000 sweeps at error and trace verbosity in separate databases. `/usr/bin/time` records wall/CPU time and peak RSS; reports must agree exactly and show 1000 observations. Trace logs are retained to inspect logging overhead. Timings depend on host load, filesystem, caching, build flags and CPU; there is no hard speed pass/fail threshold. SQLite microbenchmarks use memory, while end-to-end replay includes disk persistence. None of these numbers measure RF sensitivity, probability of detection, real USB throughput, or full-band revisit time. Replay overhead also differs from live tile logging.

## High verbosity

```sh
./bin/sentinel run --start-mhz 902 --stop-mhz 928 --sweeps 3 --capture --log-level trace --workdir runtime
# Or use -v (debug), -vv (trace), starting from the default info level.
./bin/sentinel doctor -vv --log-file doctor.jsonl
```

Operational runs create `runs/SESSION/diagnostics.jsonl`. `--log-file FILE` overrides that destination; its parent must exist. Startup records preceding session creation appear on stderr unless an explicit file is supplied. `doctor` and `report` log to stderr unless given an explicit file. Each JSON record has UTC, monotonic elapsed milliseconds, PID, sequence, level, component, message and a truncation flag. `run.json` also records total elapsed milliseconds. Levels: error, warn, info (default), debug and trace. Errors in log writes/rotation/close produce a nonzero exit rather than silent loss. Help/argument parse errors use the CLI's stderr diagnostics before logging starts.

The diagnostic file rotates at 8 MiB, retaining one `.1` backup (approximately 16 MiB total). This bound applies to diagnostic files only: events, captures and externally redirected stderr have separate storage costs. Use a distinct explicit log path per process; multi-process rotation of one shared file is unsupported. Records are flushed after each write but not fsynced per record; abrupt power loss can lose recent diagnostics. Long messages are truncated at 1535 bytes. No raw IQ or per-USB-callback logging occurs; main-thread receive summaries avoid introducing callback I/O stalls. Trace logging still has CPU/I/O cost and can affect revisit timing. Diagnostic paths and filenames may contain private information; treat logs accordingly.

## Physical receiver gate

After software qualification, use the production executable on the target machine to record known-emitter narrow-band scans, full requested-span revisit timing, capture sample counts, overload/gain behavior and controlled USB disconnect/interrupt recovery. Retain trace logs and manifests. Compare detections with an independent known reference before making sensitivity or classification claims. There is no validated device-identity model in this build.
