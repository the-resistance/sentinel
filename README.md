# Sentinel

Signal detection, trace logging, and passive protocol analysis using HackRF One and lightweight C-based tools on Kali Linux.

## Project Structure

```
sentinel/
├── src/                    # Core C source files
│   ├── main.c              # Entry point, orchestrates scan phases
│   ├── sweep.c/.h          # Frequency sweep logic
│   ├── logger.c/.h         # File and DB logging functions
│   ├── signal_proc.c/.h    # Signal analysis, filtering, PCAP triggers
│   ├── band_filter.c/.h    # Band exclusion logic
│   ├── db.c/.h             # SQLite interaction layer
│   └── utils.c/.h          # Timing, UUID gen, helpers
│
├── data/                   # Reference and config data
│   ├── protocols.def       # Known protocol definitions
│   ├── bands.exclude       # Frequencies to skip (FM, HAM, etc.)
│   ├── watchlist.csv       # MAC or UUIDs of interest
│   └── schema.sql          # SQLite DB schema
│
├── scripts/                # Utility and control scripts
│   ├── setup_sentinel.sh   # Environment setup and dependency install
│   ├── run_monitor.sh      # Wrapper: manages sweep and deep trace phases
│   └── sync_logs.sh        # Optional: uploads logs to remote if configured
│
├── build/                  # Compiled binaries
│   └── sentinel            # Final binary after `make`
│
├── logs/                   # Output sweep logs and summaries
│   └── scan_*.log
│
├── captures/               # PCAPs generated on signal match
│   └── hit_<freq>_<ts>.pcap
│
├── Makefile                # Build instructions
└── README.md               # This file
```

## Setup

Run the setup script to install all dependencies:

```bash
chmod +x scripts/setup_sentinel.sh
sudo ./scripts/setup_sentinel.sh
```

The script will install:

- HackRF tools (hackrf, libhackrf)
- GNU Radio + gr-osmosdr
- SQLite3 + dev headers
- Build essentials
- Inspectrum, aircrack-ng (optional)
- Python3 and pip (for optional tooling)

## Building the Project

From the project root:

```bash
make clean && make
```

Resulting binary: `build/sentinel`

## Running

Use the wrapper script to begin scan:

```bash
chmod +x scripts/run_monitor.sh
./scripts/run_monitor.sh
```

This handles quick scan, logs hits, and triggers deeper analysis on repeating signals.

## Logging

- All sweep logs saved under `logs/`
- PCAPs stored in `captures/`
- Signals tagged with UUIDs
- SQLite DB automatically populated via `src/db.c`

## Configuration

Edit `data/bands.exclude` to omit always-on signals (e.g., FM radio).
Add MACs or UUIDs of interest to `data/watchlist.csv`.
Protocols matched using `data/protocols.def`.

## Versioning

Version info is embedded in `main.c` and printed at runtime.
Git commit hashes may also be referenced during build.

## Notes

- Designed for HackRF One. SDR must be connected and detected via `hackrf_info`.
- Persistent logs and DB assumed under `/root/sentinel/` for Kali Live use.
- This is a passive scanning system — no active jamming, injection, or replay.