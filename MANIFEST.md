# SENTINEL SYSTEM MANIFEST

> Version: v1.0.0  
> Layout: Functional Role Map  
> Target OS: Kali Linux x86_64 (Live or Persistent)  

---

## Core Shell Scripts

- `run_monitor.sh` — Main orchestrator and runtime interface
- `quick_sweep.sh` — Initial quickband signal detection pass
- `deep_trace.sh` — Targeted capture sweep on detected signals
- `signal_watchdog.sh` — Background daemon to trigger deep traces
- `summarize_trace.sh` — MAC/vendor extraction and classification
- `setup_sentinel.sh` — Installs all packages, creates structure
- `init_db.sh` — Initializes SQLite schema and tables

---

## Data Definitions

- `data/protocols.def` — Spectrum protocol signatures (merge safe)
- `data/band_exclude.def` — Static bands to ignore during scan
- `data/band_focus.def` — Priority bands to track first
- `data/watchlist.def` — UUID/MACs to watch and correlate

---

## Source Code

- `src/main.c` — Entry logic (scan, match, report flow)
- `src/logger.c` / `src/logger.h` — Persistent logging utility
- `src/sweep.c` / `src/sweep.h` — Frequency sweeping logic
- `src/trace.c` / `src/trace.h` — Deep capture invocation
- `src/sql.c` / `src/sql.h` — SQLite write interface
- `src/defs.h` — Constants, versioning, and enums

---

## Database Files

- `db/sentinel.db` — SQLite persistent record of all detections
- `db/schema.sql` — Schema source (auto-executed by `init_db.sh`)

---

## Logs and Captures

- `logs/scan_log_*.txt` — Timestamped sweep logs
- `pcap_hits/deeptrace_<freq>_<timestamp>.pcap` — Raw captures
- `output/trace_index.csv` — MAC + vendor + hit frequency
- `output/mac_lookup.csv` — IEEE OUI snapshot

---

## Misc Support

- `Makefile` — Compilation routine (src → bin/sentinel)
- `bin/sentinel` — Optional compiled binary if built
- `README.md` — Overview and usage instructions
- `MANIFEST.md` — This file

---

## Notes

- All shell scripts assume execution from project root.
- `chmod +x *.sh` should be run after `git clone`.
- All PCAPs and DBs timestamped and auto-versioned.