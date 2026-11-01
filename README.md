# Sentinel native C runtime

Sentinel is a receive-only HackRF spectrum survey, recurrence log, CS8 capture and waveform descriptor pipeline. The supported executable is native C11: no Python interpreter, subprocess scanner, or sibling repository is required. Historical C prototypes under `src/` and the Python experiments in Sentinel-RF are reference implementations, excluded from this build.

## Build and verify

On Debian/Ubuntu, install `build-essential libsqlite3-dev libhackrf-dev`, then:

```sh
make
make test
make sanitize
./bin/sentinel doctor
```

`BACKEND=offline` builds replay/analysis without libhackrf; live operations then fail explicitly. `doctor` reports build dependencies; it does not open or validate hardware. `bin/sentinel.out` and `bin/sentinel_v1.0.1.out` are compatibility copies of the same native executable.

## Use

```sh
# Finite survey, then capture up to 10 detected targets and describe their waveforms
./bin/sentinel run --start-mhz 902 --stop-mhz 928 --sweeps 3 --capture --workdir runtime
# Continuous survey until SIGINT/SIGTERM; no automatic captures in this mode
./bin/sentinel scan --start-mhz 902 --stop-mhz 928 --sweeps 0 --workdir runtime
# Replay hackrf_sweep CSV without hardware; never starts live capture
./bin/sentinel run --replay sweep.csv --workdir replay-runtime
./bin/sentinel analyze --input iq.cs8 --sample-rate 8000000 --freq-mhz 915 --workdir analysis-runtime
./bin/sentinel analyze --captures-dir captures --workdir analysis-runtime
./bin/sentinel report --workdir runtime
```

`run-monitor.sh` starts continuous native scanning. `setup_sentinel.sh` builds and reports dependency versions without installing system packages. Run `./bin/sentinel --help` for options. Frequency flags use MHz; sample rates and bin widths use Hz. Gain is explicit (`--lna` 0..40 in steps of 8, `--vga` 0..62 in steps of 2). RF amplification and antenna power remain off. The default requested survey range is 1..6000 MHz; choose ranges appropriate to your receiver, antenna, environment and applicable rules. Scanning is sequential, not simultaneous coverage of that span.

## What the results mean

The live adapter calls libhackrf directly, retunes 20 MS/s tiles, discards 32768 settling sample pairs, and estimates power from overlapping Hann-windowed FFTs. It retains the central 75% of each tile. FFT size is selected from powers of two, 64..16384; actual bin width can differ from the requested width. This is a host-side stepped survey, not firmware sweep mode. Full-span latency, coverage, sensitivity, calibration and dropped-sample behavior require physical receiver testing. Brief or intermittent signals may be missed between visits. No instrument can promise detection of every signal.

The detector uses a local noise estimate and a configurable contrast threshold (`--margin-db`, default 10). Frequency/width groups accumulate observations across sweeps; duplicate bins in one timestamp group do not add extra recurrence hits. These groups are candidate recurring emissions, not device identities. Live timestamps use UTC; replay timestamps retain an unspecified receiver-local time scope and are not mixed with UTC groups.

CS8 descriptors include 99% occupied bandwidth, spectral flatness, envelope crest, fourth moment, clipping, DC fraction and a 32-value normalized power shape. They are labeled `waveform_descriptor_v1_c`, retain DC, and intentionally differ from earlier Python signatures. Analysis uses at most the first 262144 sample pairs and a 4096-point FFT. Classification is `unknown` and device identity is null: no trained classifier, protocol decoder or validated transmitter fingerprint is supplied. RF observations alone do not establish ownership, an agency's involvement, interception, or the presence of a GPS tracker. A receive-only GPS unit may not transmit.

## Outputs and failures

Each operation writes `WORKDIR/runs/SESSION/`: `run.json`, `spectrum_report.csv`, scan `events.jsonl`, optional `analysis.jsonl`, and optional `captures/*.cs8` plus `.meta` sidecars. `WORKDIR/sentinel.sqlite` preserves sessions, raw rows, input errors, groups, observations and sweep coverage reports. Native schema version 1 has a separate application identifier and refuses incompatible/unknown populated databases; use a fresh work directory for Python-era results. There is no automatic migration or cryptographic evidence hashing.

Sidecars use `key=value`, including `freq_mhz`, `sample_rate`, `samples`, `status=complete`, and `sample_format=cs8`. Missing metadata can be replaced by explicit frequency/sample-rate flags. Incomplete, duplicate recognized keys, unsupported sample format, odd/short files and sample-count mismatches are rejected. File inputs must be regular files, not pipes/devices. Capture memory is bounded to 20 million IQ pairs; analysis is bounded independently. Reports, captures and manifests use temporary files and rename, with file flushing before publication; this is not a transaction across every output file or a guarantee against power loss.

Exit codes: **0** complete, **1** operational failure, **2** partial results or invalid arguments, **130** interrupted. Malformed replay rows and failed individual captures are retained as errors and produce partial status. Empty/unusable input and receiver/database failures produce failure status. A valid survey with no detections is complete. SIGINT/SIGTERM request cleanup, session finalization and lock release. A per-user receiver lock prevents cooperating Sentinel processes from sharing the receiver; it cannot lock out unrelated SDR software. Receive waits have timeouts; libhackrf stop/close calls themselves depend on the library/USB stack returning.

## Validation and remaining gate

`make test` checks parsing (including 2000 malformed strings), DSP fixtures, recurrence/SQLite behavior, CLI errors, partial outputs, interruption/lock cleanup, simulated scans/captures, and the real adapter against a controlled libhackrf API shim (start/stop failures, timeout and late callback). Production replay also runs with an empty executable search path. `make sanitize` adds AddressSanitizer/UndefinedBehaviorSanitizer. These checks do not replace live hardware validation. Before deployment, verify receiver open/configuration, narrow- and full-span scans against known emitters, gain/overload behavior, capture duration/metadata, USB disconnects and interrupt cleanup on the target machine.

## Verbose diagnostics and executable test bundle

Use `--log-level trace` (or `-vv`) for detailed operation/timing records. Each run writes rotating `diagnostics.jsonl`; `--log-file FILE` selects an explicit destination. `make benchmark` measures native CPU/SQLite throughput, `bash tests/run-suite.sh` runs correctness plus performance qualification, and `make package` creates an executable Linux test bundle. See [docs/TESTING.md](docs/TESTING.md) for metrics, output locations, logging limits and physical receiver checks. GitHub Actions uploads the Linux bundle and performance logs for each successful compiler job. Version history is in [CHANGELOG.md](CHANGELOG.md).
