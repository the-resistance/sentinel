# SENTINEL RF DETECTION SYSTEM — MANIFEST
# Version:     v1.0.1
# Author:      Kevin / System Architect
# Description: Declarative list of all files, grouped by role.

---

## 🔧 Build System
- `makefile` — Compiles all C modules into `bin/sentinel_v1.0.1.out`
- `setup_sentinel.sh` — Installs dependencies, builds project

## 🚦 Runtime Execution
- `run-monitor.sh` — Orchestrates scan phases, triggers decode
- `bin/sentinel_v1.0.1.out` — Main compiled binary
- `bin/rfdash` — Optional ncurses dashboard (built via `make dashboard`)

## 📁 src/ — Core Modules (all versioned)
- `main.c` — Binary entrypoint
- `scanner.c/.h` — Primary sweep/scanning logic
- `sweep.c/.h` — Sweep control (range stepping, timing)
- `device.c/.h` — HackRF device abstraction
- `signal_processor.c/.h` — Signal logic (RSSI, thresholds)
- `band_filter.c/.h` — Band exclusion based on config
- `logger.c/.h` — Writes to `live_feed.log` and `signal_log.db`
- `uuid_utils.c/.h` — Generates UUIDs for hits
- `signal_map.c/.h` — Maps decoded signals to protocol IDs
- `matcher.c` — Lightweight signature/protocol match

## 📁 scripts/ — Shell Layer
- `on_hit_decode.sh` — Triggered post-match decoder (optional)
- `clean_unused.sh` — Deletes deprecated files (optional)

## 📁 db/
- `init_db.sh` — SQLite3 schema initializer for `signal_log.db`

## 📁 data/
- `exclude_bands.txt` — Bands to skip during scan
- `bands_tactical.txt` — Tactical band priorities
- `bands_watchlist.txt` — Band alert list
- `protocols.def` — Fingerprint map for matcher.c

## 📁 logs/ & captures/
- Created at runtime, holds logs and PCAP files

---

## 🗑 Removed Files (v1.0.1 Cleanup)
- `scripts/analyze_heatmap.sh` — not linked
- `scripts/restore_snapshot.sh` — not linked
- `src/version.h` — unused header
