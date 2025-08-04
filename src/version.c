// version.c — Build metadata string provider

#include "version.h"

const char* get_build_info() {
    return "Sentinel " SENTINEL_VERSION " built on " BUILD_DATE " at " BUILD_TIME;
}
