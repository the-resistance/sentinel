// version.c — Build metadata provider for Sentinel
// Linked into runtime for version verification

#include "version.h"

const char* get_build_info() {
    return "Sentinel RF Scanner " SENTINEL_VERSION " (built " BUILD_DATE " " BUILD_TIME ")";
}
