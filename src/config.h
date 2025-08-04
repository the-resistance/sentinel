// config.h — Sentinel Scan Configuration
// Version: 1.0.0
// Author: Kevin / System Architect

#ifndef CONFIG_H
#define CONFIG_H

#define RSSI_MIN_DBM   -80.0   // ⬅️ updated from -65
#define RSSI_MAX_DBM   -20.0
#define DWELL_TIME_US  200000  // 200ms dwell per frequency
#define STEP_SIZE_MHZ  0.5     // freq increment per scan step

#endif
