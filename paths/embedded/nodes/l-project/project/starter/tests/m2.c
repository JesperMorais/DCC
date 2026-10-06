/* Milestone 2: the event loop. FIFO in, one CSV row per period out. */
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "harness.h"

static struct row rows[MAX_ROWS];

static void creates_fifo_logdir_and_header(void) {
    write_conf(100, NULL);
    fake_temp("iio", "21000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    struct stat st;
    CHECKM(stat("sensor.fifo", &st) == 0 && S_ISFIFO(st.st_mode), "sensor.fifo should be a FIFO");
    char *text = read_file("logs/sensord.csv");
    CHECKM(text && strncmp(text, "tick,temp,count,min,mean,max\n", 29) == 0, "the first line must be the header");
    free(text);
    CHECKM(daemon_alive(), "sensord should keep running");
}

static void refuses_a_fifo_path_that_is_a_regular_file(void) {
    write_conf(100, NULL);
    fake_temp("iio", "1", "1", NULL);
    write_file("sensor.fifo", "I'm a regular file\n");
    start_daemon();
    CHECKM(WAIT_UNTIL(!daemon_alive(), 2000), "sensord should exit with an error");
    CHECK(daemon_exit_code() == 1);
    char *err = read_file("err.txt");
    CHECKM(err && strstr(err, "sensor.fifo"), "the error should name the path");
    free(err);
}

static void one_row_per_period(void) {
    write_conf(50, NULL);
    fake_temp("iio", "21000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    sleep_ms(600);
    int n = load_csv("logs/sensord.csv", rows, MAX_ROWS);
    kill_daemon();
    /* 600 ms at 50 ms is 12 periods. The host isn't real-time, so be generous. */
    CHECKM(n >= 4 && n <= 14, "expected about 12 rows after 600 ms, got %d", n);
    for (int i = 0; i < n; i++) {
        CHECKM(strcmp(rows[i].temp, "21.000") == 0, "row %d: temp \"%s\", expected 21.000", i + 1, rows[i].temp);
        CHECKM(rows[i].count == 0 && !*rows[i].min && !*rows[i].mean && !*rows[i].max,
               "row %d: no samples, so count 0 and empty min/mean/max", i + 1);
        if (i > 0) CHECKM(rows[i].tick > rows[i - 1].tick, "ticks must increase");
    }
    if (n > 0) CHECKM(rows[0].tick >= 1 && rows[0].tick <= 2, "the first tick is 1, got %ld", rows[0].tick);
}

static void samples_are_aggregated_per_period(void) {
    write_conf(200, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    int fd = fifo_open(1000);
    fifo_send(fd, "10\n20\n30\n");
    CHECK(WAIT_UNTIL(load_csv("logs/sensord.csv", rows, MAX_ROWS) > 0 &&
                         total_count(rows, csv_rows("logs/sensord.csv")) == 3, 1500));
    int n = load_csv("logs/sensord.csv", rows, MAX_ROWS);
    close(fd);
    kill_daemon();
    CHECKM(total_count(rows, n) == 3, "three samples were sent");
    for (int i = 0; i < n; i++) {
        if (rows[i].count == 3) {
            CHECK(strcmp(rows[i].min, "10.00") == 0);
            CHECK(strcmp(rows[i].mean, "20.00") == 0);
            CHECK(strcmp(rows[i].max, "30.00") == 0);
        }
    }
}

static void decimals_and_signs(void) {
    write_conf(100, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    int fd = fifo_open(1000);
    fifo_send(fd, "-1.5\n+4.25\n 2 \r\n");
    CHECK(WAIT_UNTIL(total_count(rows, load_csv("logs/sensord.csv", rows, MAX_ROWS)) == 3, 1500));
    int n = load_csv("logs/sensord.csv", rows, MAX_ROWS);
    close(fd);
    kill_daemon();
    for (int i = 0; i < n; i++)
        if (rows[i].count == 3) {
            CHECK(strcmp(rows[i].min, "-1.50") == 0);
            CHECK(strcmp(rows[i].mean, "1.58") == 0);
            CHECK(strcmp(rows[i].max, "4.25") == 0);
        }
}

static void a_line_split_across_writes(void) {
    write_conf(50, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    int fd = fifo_open(1000);
    fifo_send(fd, "4");
    sleep_ms(150); /* several ticks pass with half a line in the pipe */
    fifo_send(fd, "2\n");
    CHECK(WAIT_UNTIL(total_count(rows, load_csv("logs/sensord.csv", rows, MAX_ROWS)) >= 1, 1500));
    sleep_ms(100);
    int n = load_csv("logs/sensord.csv", rows, MAX_ROWS);
    close(fd);
    kill_daemon();
    CHECKM(total_count(rows, n) == 1, "\"4\" + \"2\\n\" is one sample, got %d", total_count(rows, n));
    for (int i = 0; i < n; i++)
        if (rows[i].count == 1) CHECKM(strcmp(rows[i].max, "42.00") == 0, "the sample is 42, got %s", rows[i].max);
}

static void garbage_lines_are_skipped(void) {
    write_conf(100, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    int fd = fifo_open(1000);
    fifo_send(fd, "abc\n\n12x\nnan\n1e999\n");
    fifo_send(fd, "123456789012345678901234567890123456789012345678901234567890123456789\n");
    fifo_send(fd, "7\n");
    sleep_ms(400);
    int n = load_csv("logs/sensord.csv", rows, MAX_ROWS);
    close(fd);
    CHECKM(daemon_alive(), "garbage must not crash sensord");
    kill_daemon();
    CHECKM(total_count(rows, n) == 1, "only \"7\" is a valid sample, got %d", total_count(rows, n));
}

static void survives_a_writer_reconnecting(void) {
    write_conf(50, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    int fd = fifo_open(1000);
    fifo_send(fd, "1\n");
    close(fd);
    sleep_ms(200);
    fd = fifo_open(1500);
    CHECKM(fd >= 0, "a new writer should be able to connect");
    fifo_send(fd, "2\n");
    CHECK(WAIT_UNTIL(total_count(rows, load_csv("logs/sensord.csv", rows, MAX_ROWS)) == 2, 1500));
    close(fd);
    kill_daemon();
}

/* Writers connect, send one sample and leave, as fast as they can. Each one
 * must find a reader: open() fails with ENXIO while nobody has the FIFO open
 * for reading, and write() fails with EPIPE if the last reader left after the
 * writer connected. Any moment without a reader is a gap that a few thousand
 * writers in a row will find. */
static void writers_never_find_the_fifo_unread(void) {
    write_conf(50, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    int sent = 0, enxio = 0, epipe = 0;
    long until = now_ms() + 500;
    while (sent < 3000 && now_ms() < until) {
        int fd = open("sensor.fifo", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0) {
            if (errno == ENXIO) enxio++;
            continue;
        }
        ssize_t n = write(fd, "1\n", 2);
        if (n == 2) sent++;
        else if (n < 0 && errno == EPIPE) epipe++;
        close(fd);
    }
    bool gap = !CHECKM(enxio == 0, "%d of the writers found nobody reading sensor.fifo (open failed with ENXIO)", enxio);
    gap |= !CHECKM(epipe == 0, "%d of the writers got EPIPE: the last reader of sensor.fifo left while they were connected", epipe);
    if (gap) return;
    CHECKM(WAIT_UNTIL(total_count(rows, load_csv("logs/sensord.csv", rows, MAX_ROWS)) == sent, 1500),
           "%d samples were sent, the log counted %d", sent, total_count(rows, load_csv("logs/sensord.csv", rows, MAX_ROWS)));
    CHECKM(daemon_alive(), "sensord should still run");
    kill_daemon();
}

static void no_busy_loop_after_the_writer_leaves(void) {
    write_conf(100, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    int fd = fifo_open(1000);
    fifo_send(fd, "5\n");
    close(fd); /* EOF on the read end, forever */
    sleep_ms(100);
    long before = daemon_cpu_ms();
    sleep_ms(800);
    long used = daemon_cpu_ms() - before;
    CHECKM(daemon_alive(), "sensord should still run");
    kill_daemon();
    CHECKM(used < 250, "sensord used %ld ms of CPU in 800 ms with nothing to do: is it spinning on EOF?", used);
}

static void temp_column_empty_when_the_channel_breaks(void) {
    write_conf(50, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    CHECK(WAIT_UNTIL(csv_rows("logs/sensord.csv") >= 1, 1000));
    unlink("iio/in_temp_raw");
    int before = csv_rows("logs/sensord.csv");
    CHECK(WAIT_UNTIL(csv_rows("logs/sensord.csv") >= before + 3, 1500));
    int n = load_csv("logs/sensord.csv", rows, MAX_ROWS);
    CHECKM(daemon_alive(), "a missing file is a bad reading, not a crash");
    kill_daemon();
    CHECK(n >= 2 && strcmp(rows[n - 1].temp, "") == 0);
    CHECK(n >= 2 && strcmp(rows[0].temp, "20.000") == 0);
}

static void appends_to_an_existing_log(void) {
    write_conf(50, NULL);
    fake_temp("iio", "20000", "0.001", NULL);
    for (int run = 0; run < 2; run++) {
        start_daemon();
        if (!CHECK(wait_ready(2000))) return;
        CHECK(WAIT_UNTIL(csv_rows("logs/sensord.csv") >= 2 * (run + 1), 1500));
        kill_daemon();
    }
    char *text = read_file("logs/sensord.csv");
    int headers = 0;
    for (char *p = text; p && (p = strstr(p, "tick,temp")); p++) headers++;
    free(text);
    CHECKM(headers == 1, "the header appears %d times, expected once", headers);
    int n = load_csv("logs/sensord.csv", rows, MAX_ROWS);
    int ones = 0;
    for (int i = 0; i < n; i++) ones += rows[i].tick == 1;
    CHECKM(ones == 2, "both runs start at tick 1");
}

void m2_tests(void) {
    RUN(creates_fifo_logdir_and_header);
    RUN(refuses_a_fifo_path_that_is_a_regular_file);
    RUN(one_row_per_period);
    RUN(samples_are_aggregated_per_period);
    RUN(decimals_and_signs);
    RUN(a_line_split_across_writes);
    RUN(garbage_lines_are_skipped);
    RUN(survives_a_writer_reconnecting);
    RUN(writers_never_find_the_fifo_unread);
    RUN(no_busy_loop_after_the_writer_leaves);
    RUN(temp_column_empty_when_the_channel_breaks);
    RUN(appends_to_an_existing_log);
}
