# Supported integration manifest

- src/main.c: compatibility launcher into Sentinel-RF/sentinel.py.
- makefile: builds bin/sentinel.out and the old binary-name alias.
- run-monitor.sh: continuous scan through the unified runtime.
- setup_sentinel.sh: dependency preflight only.
- scripts/scripts/make_snapshot.sh: source snapshot with checked tar status.
- scripts/on_hit_decode.sh: explicit unsupported-decoder result.
- scripts/clean_unused.sh: does not delete required headers.
- Other src files, rfdash.c, db/init_db.sh and data definitions: historical
  prototypes, not linked or called by the supported runtime.
- a.zip: historical binary archive, not executed or used as source of truth.

The validated data contract, reports and lifecycle are in Sentinel-RF. See its
docs/INTEGRATION_REVIEW.md for reviewed files, failures and validation scope.

