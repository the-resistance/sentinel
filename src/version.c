// version.c — Build Metadata

#include "version.h"

const char* get_build_info(void) {
    return "Sentinel RF Scanner v1.0.3 — (Build: " __DATE__ " " __TIME__ ")";
}
