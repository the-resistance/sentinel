// rfdash.c
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MAX_HITS 10

typedef struct {
    char freq[16];
    char strength[8];
    char timestamp[32];
} Hit;

Hit hits[MAX_HITS];
int total_hits = 0;

void draw_dashboard() {
    clear();
    mvprintw(0, 2, "Sentinel RF Dashboard");
    mvprintw(1, 2, "Scan Phase: %s", getenv("SCAN_PHASE") ? getenv("SCAN_PHASE") : "UNKNOWN");
    mvprintw(2, 2, "Hits (last %d):", MAX_HITS);

    for (int i = 0; i < MAX_HITS; i++) {
        if (strlen(hits[i].freq) > 0) {
            mvprintw(4 + i, 4, "[%s] %s dB  @ %s", hits[i].freq, hits[i].strength, hits[i].timestamp);
        }
    }

    mvprintw(15, 2, "Total hits: %d", total_hits);
    refresh();
}

void shift_hits() {
    for (int i = MAX_HITS - 1; i > 0; i--) {
        hits[i] = hits[i - 1];
    }
}

void add_hit(const char *freq, const char *strength) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char ts[32];
    strftime(ts, 32, "%H:%M:%S", tm_info);

    shift_hits();
    snprintf(hits[0].freq, 16, "%s", freq);
    snprintf(hits[0].strength, 8, "%s", strength);
    snprintf(hits[0].timestamp, 32, "%s", ts);
    total_hits++;
}

int main() {
    initscr();
    noecho();
    curs_set(FALSE);

    while (1) {
        FILE *fp = fopen("logs/live_feed.log", "r");
        if (fp) {
            char line[128];
            while (fgets(line, sizeof(line), fp)) {
                char freq[16], strength[8];
                if (sscanf(line, "%15s %7s", freq, strength) == 2) {
                    add_hit(freq, strength);
                }
            }
            fclose(fp);
            remove("logs/live_feed.log"); // clear processed
        }

        draw_dashboard();
        usleep(300000);  // 300ms
    }

    endwin();
    return 0;
}
