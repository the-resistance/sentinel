// version.h — Sentinel RF Scanner
// Embedded build metadata for runtime display
// Version: 1.0.1

#ifndef VERSION_H
#define VERSION_H

#define SENTINEL_VERSION "v1.0.1"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

const char* get_build_info();

#endif
