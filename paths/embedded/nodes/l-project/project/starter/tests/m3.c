/* Milestone 3: signals. SIGTERM/SIGINT drain and flush, SIGHUP reloads. */
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "harness.h"

static struct row rows[MAX_ROWS];

static bool stderr_has(const char *needle) {
    char *err = read_file("err.txt");
    bool found = err && strstr(err, needle);
    free(err);
    return found;
}

static void sigterm_exits_cleanly(void) {
    write_conf(50, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    CHECK(WAIT_UNTIL(csv_rows("logs/sensord.csv") >= 2, 1500));
    CHECKM(stop_daemon(SIGTERM, 1500) == 0, "SIGTERM should end sensord with exit code 0");
}

static void sigint_exits_cleanly(void) {
    write_conf(50, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    CHECKM(stop_daemon(SIGINT, 1500) == 0, "SIGINT (Ctrl-C) should end sensord with exit code 0");
}

static void shutdown_drains_the_fifo_and_writes_a_final_row(void) {
    write_conf(10000, NULL); /* no tick will happen during this test */
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    int fd = fifo_open(1000);
    fifo_send(fd, "5\n6\n7\n");
    /* The samples are in the pipe; sensord may not have read them yet. */
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
    close(fd);
    int n = load_csv("logs/sensord.csv", rows, MAX_ROWS);
    if (!CHECKM(n == 1, "expected exactly one (final) row, got %d", n)) return;
    CHECK(rows[0].tick == 1);
    CHECK(strcmp(rows[0].temp, "20.000") == 0);
    CHECKM(rows[0].count == 3, "the final row holds the samples still in the pipe, got %d", rows[0].count);
    CHECK(strcmp(rows[0].min, "5.00") == 0 && strcmp(rows[0].mean, "6.00") == 0 && strcmp(rows[0].max, "7.00") == 0);
}

static void final_row_continues_the_tick_count(void) {
    write_conf(50, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    CHECK(WAIT_UNTIL(csv_rows("logs/sensord.csv") >= 3, 1500));
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
    int n = load_csv("logs/sensord.csv", rows, MAX_ROWS);
    if (!CHECK(n >= 4)) return;
    CHECKM(rows[n - 1].tick == rows[n - 2].tick + 1, "the final row is tick %ld after tick %ld",
           rows[n - 1].tick, rows[n - 2].tick);
}

static void sighup_reloads_the_period(void) {
    write_conf(5000, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    sleep_ms(200);
    CHECKM(csv_rows("logs/sensord.csv") == 0, "no tick yet at a 5 s period");
    write_conf(50, NULL);
    kill(daemon_pid_for_tests(), SIGHUP);
    CHECKM(WAIT_UNTIL(csv_rows("logs/sensord.csv") >= 4, 1500), "after SIGHUP the 50 ms period should apply");
    CHECKM(daemon_alive(), "SIGHUP must not kill sensord");
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
}

static void sighup_reloads_the_channel_path(void) {
    write_conf(50, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    fake_temp("iio2", "30000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    CHECK(WAIT_UNTIL(csv_rows("logs/sensord.csv") >= 1, 1500));
    write_file("sensord.conf", "fifo = sensor.fifo\nsysfs = iio2\nlog_dir = logs\nperiod_ms = 50\n");
    kill(daemon_pid_for_tests(), SIGHUP);
    sleep_ms(300);
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
    int n = load_csv("logs/sensord.csv", rows, MAX_ROWS);
    if (!CHECK(n >= 3)) return;
    CHECK(strcmp(rows[0].temp, "20.000") == 0);
    CHECKM(strcmp(rows[n - 1].temp, "30.000") == 0, "after the reload the temperature comes from iio2");
}

static void broken_reload_keeps_the_old_config(void) {
    write_conf(50, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    CHECK(WAIT_UNTIL(csv_rows("logs/sensord.csv") >= 1, 1500));
    write_file("sensord.conf", "fifo = sensor.fifo\nthis is not a config\n");
    kill(daemon_pid_for_tests(), SIGHUP);
    sleep_ms(150);
    int before = csv_rows("logs/sensord.csv");
    CHECKM(daemon_alive(), "a broken config on reload must not stop sensord");
    CHECKM(WAIT_UNTIL(csv_rows("logs/sensord.csv") >= before + 3, 1500), "logging goes on at the old period");
    CHECKM(stderr_has("sensord.conf:2:"), "the reload error names the line, like at startup");
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
}

static void signals_arriving_together(void) {
    write_conf(50, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    CHECK(WAIT_UNTIL(csv_rows("logs/sensord.csv") >= 1, 1500));
    for (int i = 0; i < 20; i++) kill(daemon_pid_for_tests(), SIGHUP);
    sleep_ms(100);
    CHECKM(daemon_alive(), "a burst of SIGHUPs must not kill sensord");
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
    CHECK(csv_rows("logs/sensord.csv") >= 1);
}

void m3_tests(void) {
    RUN(sigterm_exits_cleanly);
    RUN(sigint_exits_cleanly);
    RUN(shutdown_drains_the_fifo_and_writes_a_final_row);
    RUN(final_row_continues_the_tick_count);
    RUN(sighup_reloads_the_period);
    RUN(sighup_reloads_the_channel_path);
    RUN(broken_reload_keeps_the_old_config);
    RUN(signals_arriving_together);
}
