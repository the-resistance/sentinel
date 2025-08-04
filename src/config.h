// config.h — Sentinel Scan Configuration
// Version: 1.1.0
// Author: Kevin / System Architect

#ifndef CONFIG_H
#define CONFIG_H

// RSSI filter thresholds
#define RSSI_MIN_DBM      -80.0
#define RSSI_MAX_DBM      -20.0

// Per-scan-mode dwell times (in microseconds)
#define DWELL_QUICK_US     15000   // 15 ms
#define DWELL_GENERAL_US   80000   // 80 ms
#define DWELL_PROTOCOL_US  120000  // 120 ms
#define DWELL_THOROUGH_US  200000  // 200 ms

// Frequency stepping
#define STEP_SIZE_MHZ      0.5

#endif
