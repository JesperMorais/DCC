/* Milestone 5: the control socket. */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "harness.h"

#define CONTROL "control = sensord.sock\n"

static bool start_with_socket(int period_ms) {
    write_conf(period_ms, CONTROL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    return wait_ready(2000);
}

/* Reads "key=value" out of a stats line. */
static long stat_value(const char *line, const char *key) {
    char pattern[32];
    snprintf(pattern, sizeof pattern, "%s=", key);
    const char *p = strstr(line, pattern);
    if (!p || (p != line && p[-1] != ' ')) return -1;
    return strtol(p + strlen(pattern), NULL, 10);
}

static void ping_pong(void) {
    if (!CHECK(start_with_socket(100))) return;
    int s = sock_connect("sensord.sock", 1000);
    if (!CHECKM(s >= 0, "cannot connect to sensord.sock")) return;
    char reply[256];
    CHECK(sock_send(s, "ping\n"));
    CHECKM(sock_line(s, reply, sizeof reply, 1000) && strcmp(reply, "pong") == 0, "got \"%s\"", reply);
    close(s);
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
}

static void stats_counts_samples_and_errors(void) {
    if (!CHECK(start_with_socket(50))) return;
    int fd = fifo_open(1000);
    fifo_send(fd, "1\n2\nbad\n3\n");
    CHECK(WAIT_UNTIL(csv_rows("logs/sensord.csv") >= 3, 1500));
    int s = sock_connect("sensord.sock", 1000);
    if (!CHECK(s >= 0)) return;
    char reply[256];
    CHECK(sock_send(s, "stats\n"));
    CHECK(sock_line(s, reply, sizeof reply, 1000));
    printf("#   stats: %s\n", reply);
    CHECKM(stat_value(reply, "samples") == 3, "samples=3 expected");
    CHECKM(stat_value(reply, "errors") == 1, "errors=1 expected");
    CHECKM(stat_value(reply, "ticks") >= 3, "ticks should be at least 3");
    CHECKM(stat_value(reply, "rotations") == 0, "rotations=0 expected");
    close(s);
    close(fd);
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
}

static void stats_counts_rotations_and_bad_readings(void) {
    write_conf(20, CONTROL "rotate_lines = 2\nkeep = 1\n");
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    CHECK(WAIT_UNTIL(csv_rows("logs/sensord.csv") >= 1 && exists("logs/sensord.csv.1"), 1500));
    unlink("iio/in_temp_raw");
    sleep_ms(150);
    int s = sock_connect("sensord.sock", 1000);
    if (!CHECK(s >= 0)) return;
    char reply[256];
    CHECK(sock_send(s, "stats\n"));
    CHECK(sock_line(s, reply, sizeof reply, 1000));
    CHECKM(stat_value(reply, "rotations") >= 1, "got \"%s\"", reply);
    CHECKM(stat_value(reply, "errors") >= 2, "failed temperature readings count as errors, got \"%s\"", reply);
    close(s);
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
}

static void several_commands_on_one_connection(void) {
    if (!CHECK(start_with_socket(100))) return;
    int s = sock_connect("sensord.sock", 1000);
    if (!CHECK(s >= 0)) return;
    char reply[256];
    CHECK(sock_send(s, "ping\nping\n"));
    CHECK(sock_line(s, reply, sizeof reply, 1000) && strcmp(reply, "pong") == 0);
    CHECK(sock_line(s, reply, sizeof reply, 1000) && strcmp(reply, "pong") == 0);
    CHECK(sock_send(s, "st"));
    sleep_ms(100);
    CHECK(sock_send(s, "ats\n"));
    CHECKM(sock_line(s, reply, sizeof reply, 1000) && stat_value(reply, "ticks") >= 0,
           "a command split across two writes, got \"%s\"", reply);
    CHECK(sock_send(s, "reboot\n"));
    CHECKM(sock_line(s, reply, sizeof reply, 1000) && strncmp(reply, "error", 5) == 0,
           "an unknown command gets a line starting with \"error\", got \"%s\"", reply);
    CHECK(sock_send(s, "ping\n"));
    CHECKM(sock_line(s, reply, sizeof reply, 1000) && strcmp(reply, "pong") == 0, "the connection stays usable");
    close(s);
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
}

static void a_silent_client_blocks_nothing(void) {
    if (!CHECK(start_with_socket(50))) return;
    int silent = sock_connect("sensord.sock", 1000);
    int half = sock_connect("sensord.sock", 1000);
    if (!CHECK(silent >= 0 && half >= 0)) return;
    CHECK(sock_send(half, "pi")); /* and never the rest */
    int before = csv_rows("logs/sensord.csv");
    int s = sock_connect("sensord.sock", 1000);
    char reply[256];
    CHECK(s >= 0 && sock_send(s, "ping\n"));
    CHECKM(s >= 0 && sock_line(s, reply, sizeof reply, 500) && strcmp(reply, "pong") == 0,
           "a third client is answered while two others idle");
    CHECKM(WAIT_UNTIL(csv_rows("logs/sensord.csv") >= before + 3, 1500), "and logging goes on");
    close(s);
    close(half);
    close(silent);
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
}

static void clients_do_not_leak_fds(void) {
    if (!CHECK(start_with_socket(100))) return;
    int s = sock_connect("sensord.sock", 1000);
    char reply[256];
    CHECK(s >= 0 && sock_send(s, "ping\n") && sock_line(s, reply, sizeof reply, 1000));
    close(s);
    sleep_ms(50);
    int before = daemon_fd_count();
    for (int i = 0; i < 60; i++) {
        s = sock_connect("sensord.sock", 1000);
        if (s < 0) break;
        if (i % 2) {
            sock_send(s, "ping\n");
            sock_line(s, reply, sizeof reply, 1000);
        }
        close(s);
    }
    CHECK(s >= 0);
    sleep_ms(100);
    int after = daemon_fd_count();
    CHECKM(after <= before + 1, "sensord had %d open fds before 60 clients and %d after", before, after);
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
}

static void stale_socket_is_replaced_and_removed_on_exit(void) {
    /* A crashed run leaves its socket file behind. */
    write_conf(100, CONTROL);
    fake_temp("iio", "20000", "0.001", NULL);
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    CHECK(WAIT_UNTIL(exists("sensord.sock"), 1000));
    kill_daemon();
    CHECK(exists("sensord.sock"));
    start_daemon();
    if (!CHECK(wait_ready(2000))) return;
    int s = sock_connect("sensord.sock", 1000);
    char reply[256];
    CHECKM(s >= 0 && sock_send(s, "ping\n") && sock_line(s, reply, sizeof reply, 1000),
           "sensord should replace a stale socket file");
    if (s >= 0) close(s);
    CHECK(stop_daemon(SIGTERM, 1500) == 0);
    CHECKM(!exists("sensord.sock"), "a clean shutdown removes the socket file");
}

static void refuses_to_delete_a_regular_file(void) {
    write_conf(100, CONTROL);
    fake_temp("iio", "20000", "0.001", NULL);
    write_file("sensord.sock", "not a socket\n");
    start_daemon();
    CHECKM(WAIT_UNTIL(!daemon_alive(), 2000), "sensord should refuse to start");
    CHECK(daemon_exit_code() == 1);
    char *err = read_file("err.txt");
    CHECKM(err && strstr(err, "sensord.sock"), "the error should name the path");
    free(err);
    char *text = read_file("sensord.sock");
    CHECKM(text && strcmp(text, "not a socket\n") == 0, "and must leave the file alone");
    free(text);
}

void m5_tests(void) {
    RUN(ping_pong);
    RUN(stats_counts_samples_and_errors);
    RUN(stats_counts_rotations_and_bad_readings);
    RUN(several_commands_on_one_connection);
    RUN(a_silent_client_blocks_nothing);
    RUN(clients_do_not_leak_fds);
    RUN(stale_socket_is_replaced_and_removed_on_exit);
    RUN(refuses_to_delete_a_regular_file);
}
