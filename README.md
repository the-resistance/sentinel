# rf-sentinel

`rf-sentinel` is a lightweight RF signal scanner and logger built for use with HackRF. It performs wideband sweeps, excludes known broadcast ranges, and records signal detections into both CSV and SQLite for persistence analysis.

## Features

- Full sweep across 1 MHz – 6 GHz
- Persistent signal logging (timestamped)
- SQLite and CSV output logging
- Ignore and tactical band filtering
- Configurable step size, dwell time, and sample rate

## Directory Layout

```
sentinel/
├── Makefile                  # Build system
├── README.md                 # This file
├── db/
│   ├── signal_log.db         # SQLite DB (runtime)
│   └── init_db.sh            # (Optional) Manual DB initializer
├── logs/
│   └── trace_index.csv       # CSV log output
├── data/
│   ├── ignore_bands.txt      # Ranges to skip
│   └── tactical_bands.txt    # Ranges of interest
├── src/
│   ├── main.c
│   ├── sweep.c/.h
│   ├── band_filter.c/.h
│   ├── logger.c/.h
│   ├── signal_map.c/.h       # (Optional/Pluggable)
│   └── version.h
```

## Build Instructions (Kali Linux Live)

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  libhackrf-dev \
  libsqlite3-dev \
  sqlite3 \
  git \
  hackrf \
  gnuradio

git clone https://github.com/the-resistance/sentinel ~/sentinel
cd ~/sentinel
make
```

## Running

```bash
./rfscan
```

### Default sweep configuration:

- Start Frequency: 1 MHz
- End Frequency: 6 GHz
- Step Size: 1 MHz
- Sample Rate: 10 MSPS
- Dwell Time: 75 ms

## Output Logs

- `logs/trace_index.csv`: Rolling signal hits
- `db/signal_log.db`: SQLite record of all signal detections
- Each row includes: timestamp, frequency, RSSI, and signal status

## Band Filtering

Two configuration files in `data/`:

- `ignore_bands.txt`: Frequencies to **skip** (e.g., FM, broadcast TV, etc.)
- `tactical_bands.txt`: Frequencies to **flag** for persistence checking

Each line:
```
<start_freq> <end_freq>
```

Example:
```
88100000 108000000
2400000000 2480000000
```

## License

This project is released for civil and diagnostic use only. No warranty or guarantees.
