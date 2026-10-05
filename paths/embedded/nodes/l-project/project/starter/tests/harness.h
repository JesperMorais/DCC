/* harness.h: black-box helpers. The tests never look inside sensord: they
 * start the binary in a fresh temp directory, feed its FIFO, send it signals,
 * talk to its socket and read the files it writes. Any structure passes.
 */
#ifndef HARNESS_H
#define HARNESS_H

#include <stdbool.h>
#include <sys/types.h>

#include "check.h"

void harness_init(void);

/* Every test runs in its own empty temp directory (the current directory). */

/* Files */
void write_file(const char *path, const char *fmt, ...);
char *read_file(const char *path); /* malloc'd, NULL if missing. free() it. */
bool exists(const char *path);
int count_lines(const char *text); /* lines starting with anything, NULL = 0 */

/* A fake IIO-style temperature channel in ./iio (or another directory). */
void fake_temp(const char *dir, const char *raw, const char *scale, const char *offset);

/* Writes ./sensord.conf: fifo, sysfs, log_dir, period_ms, plus extra lines. */
void write_conf(int period_ms, const char *extra);

/* Running sensord. stdout goes to ./out.txt, stderr to ./err.txt. */
int run_once(const char *arg1, const char *arg2, int timeout_ms); /* exit code, -1 on crash/timeout */
pid_t start_daemon(void);       /* sensord sensord.conf, in the background */
bool daemon_alive(void);
pid_t daemon_pid_for_tests(void); /* to send it signals with kill() */
bool wait_ready(int timeout_ms); /* FIFO accepts a writer and logs/sensord.csv exists */
int stop_daemon(int sig, int timeout_ms); /* sends sig, waits; exit code, -1 if it didn't exit cleanly */
void kill_daemon(void);         /* SIGKILL, no questions asked */
int daemon_exit_code(void);     /* after it exited by itself: its exit code, -1 if still running or crashed */

/* The sensor FIFO */
int fifo_open(int timeout_ms);  /* a writable fd for ./sensor.fifo, -1 if no reader appears */
void fifo_send(int fd, const char *text);

/* The CSV log */
#define MAX_ROWS 512
struct row {
    long tick;
    char temp[32];
    int count;
    char min[32], mean[32], max[32];
};
/* Parses a CSV log. Returns the number of data rows, or -1 if the file is
 * missing or the first line isn't the header. Malformed rows are -2. */
int load_csv(const char *path, struct row *rows, int max);
int csv_rows(const char *path); /* data rows, or a negative error as above */
int total_count(const struct row *rows, int n);

/* /proc introspection of the running daemon */
long daemon_cpu_ms(void);
int daemon_fd_count(void);

/* The control socket */
int sock_connect(const char *path, int timeout_ms);
bool sock_send(int fd, const char *text);
bool sock_line(int fd, char *buf, int len, int timeout_ms); /* one reply line, without '\n' */

/* Time */
void sleep_ms(int ms);
long now_ms(void);

/* Polls cond every 10 ms until it's true or ms have passed. */
#define WAIT_UNTIL(cond, ms) ({                                       \
    long until_ = now_ms() + (ms);                                    \
    bool ok_;                                                         \
    while (!(ok_ = (cond)) && now_ms() < until_) sleep_ms(10);        \
    ok_; })

#endif
