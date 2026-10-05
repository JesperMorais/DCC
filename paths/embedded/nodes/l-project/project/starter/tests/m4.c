/* Milestone 4: log rotation. */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "harness.h"

static struct row rows[MAX_ROWS], older[MAX_ROWS];

static void rotates_and_keeps_n_files(void) {
    write_conf(20, "rotate_lines = 5\nkeep = 2\n");
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    CHECKM(WAIT_UNTIL(exists("logs/sensord.csv.2") && csv_rows("logs/sensord.csv") >= 2, 2500),
           "after ~15 rows at 5 rows per file, sensord.csv.2 should exist");
    sleep_ms(250);
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
    CHECKM(!exists("logs/sensord.csv.3"), "keep = 2: only .1 and .2 are kept");
    int n2 = load_csv("logs/sensord.csv.2", older, MAX_ROWS);
    int n1 = load_csv("logs/sensord.csv.1", rows, MAX_ROWS);
    CHECKM(n2 == 5, "sensord.csv.2 has %d data rows after its header, expected 5", n2);
    CHECKM(n1 == 5, "sensord.csv.1 has %d data rows after its header, expected 5", n1);
    if (n1 == 5 && n2 == 5) CHECKM(older[4].tick < rows[0].tick, ".2 is older than .1");
    long last_of_1 = n1 > 0 ? rows[n1 - 1].tick : 0;
    int n = load_csv("logs/sensord.csv", rows, MAX_ROWS);
    CHECKM(n >= 1 && n <= 5, "the current file has %d rows, expected 1 to 5", n);
    if (n >= 1) CHECKM(rows[0].tick > last_of_1, "the current file continues after .1");
}

static void zero_means_never_rotate(void) {
    write_conf(20, "rotate_lines = 0\n");
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    CHECK(WAIT_UNTIL(csv_rows("logs/sensord.csv") >= 15, 2500));
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
    CHECK(!exists("logs/sensord.csv.1"));
}

static void every_file_starts_with_the_header(void) {
    write_conf(20, "rotate_lines = 3\nkeep = 4\n");
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    CHECK(WAIT_UNTIL(exists("logs/sensord.csv.4"), 2500));
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
    const char *names[] = { "logs/sensord.csv", "logs/sensord.csv.1", "logs/sensord.csv.2", "logs/sensord.csv.3", "logs/sensord.csv.4" };
    for (int i = 0; i < 5; i++) {
        int n = csv_rows(names[i]);
        CHECKM(n >= 0, "%s should start with the header", names[i]);
        if (i > 0) CHECKM(n == 3, "%s has %d rows, expected 3", names[i], n);
    }
    CHECK(!exists("logs/sensord.csv.5"));
}

static void existing_rows_count_towards_the_limit(void) {
    write_conf(50, "rotate_lines = 5\nkeep = 3\n");
    fake_temp("iio", "20000", "0.001", NULL);
    mkdir("logs", 0755);
    write_file("logs/sensord.csv", "tick,temp,count,min,mean,max\n101,1.000,0,,,\n102,1.000,0,,,\n103,1.000,0,,,\n");
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    CHECKM(WAIT_UNTIL(exists("logs/sensord.csv.1"), 2000), "3 old rows + 2 new ones fill the first file");
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
    int n = load_csv("logs/sensord.csv.1", rows, MAX_ROWS);
    CHECKM(n == 5, "sensord.csv.1 has %d rows, expected 5", n);
    if (n == 5) {
        CHECK(rows[0].tick == 101 && rows[2].tick == 103);
        CHECKM(rows[3].tick == 1 && rows[4].tick == 2, "then the new run's ticks 1 and 2");
    }
}

static void no_samples_lost_across_rotations(void) {
    write_conf(20, "rotate_lines = 4\nkeep = 9\n");
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    int fd = fifo_open(1000);
    char line[16];
    for (int i = 1; i <= 30; i++) {
        snprintf(line, sizeof line, "%d\n", i);
        fifo_send(fd, line);
        sleep_ms(15);
    }
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
    close(fd);
    int total = 0, files = 0;
    char name[64];
    for (int i = 0; i <= 9; i++) {
        if (i == 0) snprintf(name, sizeof name, "logs/sensord.csv");
        else snprintf(name, sizeof name, "logs/sensord.csv.%d", i);
        if (!exists(name)) continue;
        int n = load_csv(name, rows, MAX_ROWS);
        if (n > 0) total += total_count(rows, n);
        files++;
    }
    CHECKM(files >= 3, "expected several rotated files, found %d", files);
    CHECKM(total == 30, "30 samples went in, the logs hold %d", total);
}

void m4_tests(void) {
    RUN(rotates_and_keeps_n_files);
    RUN(zero_means_never_rotate);
    RUN(every_file_starts_with_the_header);
    RUN(existing_rows_count_towards_the_limit);
    RUN(no_samples_lost_across_rotations);
}
