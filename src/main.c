/* Compatibility launcher: real acquisition lives in Sentinel-RF. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    char script[PATH_MAX], executable[PATH_MAX];
    const char *root = getenv("SENTINEL_RF_ROOT");
    if (root && *root) {
        if (snprintf(script, sizeof(script), "%s/sentinel.py", root) >= (int)sizeof(script)) {
            fputs("Sentinel: runtime path too long\n", stderr); return 2;
        }
    } else {
        ssize_t size = readlink("/proc/self/exe", executable, sizeof(executable)-1);
        if (size < 0 || size >= (ssize_t)sizeof(executable)-1) {
            fputs("Sentinel: set SENTINEL_RF_ROOT to your Sentinel-RF checkout\n", stderr); return 2;
        }
        executable[size] = '\0';
        char *slash = strrchr(executable, '/');
        if (!slash) return 2;
        *slash = '\0';
        if (snprintf(script, sizeof(script), "%s/../../Sentinel-RF/sentinel.py", executable) >= (int)sizeof(script)) return 2;
    }
    if (access(script, R_OK) != 0) {
        fprintf(stderr, "Sentinel: runtime unavailable at %s; set SENTINEL_RF_ROOT\n", script); return 2;
    }
    char **args = calloc((size_t)argc+10, sizeof(*args));
    if (!args) { perror("Sentinel: allocation"); return 1; }
    const char *python = getenv("PYTHON");
    args[0] = (char *)(python && *python ? python : "python3");
    args[1] = script;
    int n = 2;
    if (argc >= 3 && strcmp(argv[1], "--mode") == 0) {
        if (strcmp(argv[2], "general") == 0) {
            args[n++] = "scan"; args[n++] = "--sweeps"; args[n++] = "0";
        } else if (strcmp(argv[2], "quick") == 0) {
            args[n++] = "run"; args[n++] = "--start-mhz"; args[n++] = "902";
            args[n++] = "--stop-mhz"; args[n++] = "928";
        } else {
            fputs("Sentinel: obsolete mode; use run/scan with explicit ranges\n", stderr);
            free(args); return 2;
        }
        for (int i=3; i<argc; ++i) args[n++] = argv[i];
    } else {
        for (int i=1; i<argc; ++i) args[n++] = argv[i];
    }
    execvp(args[0], args);
    fprintf(stderr, "Sentinel: cannot start Python runtime: %s\n", strerror(errno));
    free(args); return 1;
}

