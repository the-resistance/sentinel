# Sentinel compatibility launcher

Real acquisition, capture, analysis and reporting are maintained in
[Sentinel-RF](https://github.com/the-resistance/Sentinel-RF).
Use its integration branch until the corresponding pull requests are merged.

Place both checkouts side by side, or set SENTINEL_RF_ROOT to the absolute
Sentinel-RF checkout path. Run from any working directory:

```bash
make
./bin/sentinel.out doctor
./bin/sentinel.out run --replay /absolute/path/sweep.csv --workdir /absolute/path/results
./bin/sentinel.out run --capture --sweeps 3 --workdir /absolute/path/results
./bin/sentinel.out report --workdir /absolute/path/results
```

The C executable is a small exec-based launcher. It does not link the incomplete
native acquisition modules or generate simulated detections. Old general mode
maps to continuous scan; old quick mode maps to a finite 902..928 MHz scan.
Obsolete protocol mode fails explicitly; use explicit scan bounds.

The remaining native source files are historical prototypes, excluded from the
supported build. They contain stub acquisition, incompatible interfaces and
non-unique frequency-derived IDs. Do not compile them together as a scanner.
The old dashboard has no producer for its legacy log format and is not integrated.
Decoder hooks fail explicitly instead of reporting successful decoding.

setup_sentinel.sh now checks dependencies without changing the system timezone,
injecting system headers, killing unrelated processes, or installing optional
software. Follow the Sentinel-RF README for dependency setup and error codes.
