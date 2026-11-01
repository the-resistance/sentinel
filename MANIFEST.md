# Supported source map

- `src/main.c`: native CLI, orchestration, locking, run status and cleanup.
- `src/native/`: CSV validation, detector/FFT descriptors, SQLite persistence, libhackrf receiver, stepped survey and capture/analysis.
- `Makefile.native`, included by `makefile`: supported build; no legacy prototype sources linked.
- `tests/test_core.c`, `tests/test_receiver.c`, `tests/fake_receiver.c`, `tests/integration.sh`: native validation.
- `run-monitor.sh`, `setup_sentinel.sh`: native shell entry points.
- Other historical sources and documentation are retained for reference. README.md defines the supported runtime and limitations.
