#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <time.h>
#include <unistd.h>

/* ---- helpers ---- */

static void sleep_ms(long ms) {
    struct timespec t = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&t, NULL);
}

static double now_ms(clockid_t clk) {
    struct timespec t;
    clock_gettime(clk, &t);
    return (double)t.tv_sec * 1e3 + (double)t.tv_nsec / 1e6;
}

static void put(int fd, const char *text) {
    if (write(fd, text, strlen(text)) < 0) perror("write");
}

static int open_fd_count(void) {
    int n = 0;
    DIR *d = opendir("/proc/self/fd");
    if (!d) return -1;
    while (readdir(d)) n++;
    closedir(d);
    return n;
}

/* The whole output file, NUL-terminated, in a static buffer. */
static char out_text[16384];
static const char *slurp(const char *path) {
    out_text[0] = '\0';
    FILE *f = fopen(path, "r");
    if (!f) return out_text;
    size_t n = fread(out_text, 1, sizeof out_text - 1, f);
    out_text[n] = '\0';
    fclose(f);
    return out_text;
}

/* Copy line `want` (0-based) of the output that starts with `prefix` into buf; returns how many such lines exist. */
static int nth_line(const char *prefix, int want, char *buf, size_t cap) {
    int count = 0;
    buf[0] = '\0';
    for (const char *p = out_text; *p;) {
        const char *nl = strchr(p, '\n');
        size_t len = nl ? (size_t)(nl - p) : strlen(p);
        if (strncmp(p, prefix, strlen(prefix)) == 0) {
            if (count == want || want < 0) snprintf(buf, cap, "%.*s", (int)len, p);
            count++;
        }
        p += len + (nl ? 1 : 0);
    }
    return count;
}

/* After `ms`, optionally write `late` to a sensor, then after `stop_ms` more, signal shutdown. */
struct script { int efd; long ms; int late_fd; const char *late; long stop_ms; };

static void *run_script(void *arg) {
    struct script *s = arg;
    sleep_ms(s->ms);
    if (s->late) put(s->late_fd, s->late);
    sleep_ms(s->stop_ms);
    uint64_t one = 1;
    if (write(s->efd, &one, sizeof one) < 0) perror("eventfd");
    return NULL;
}

/* Is the averages part of a T line ("T 7 20.0 200.0" → "20.0 200.0") equal to `want`? */
static int tick_avgs_are(const char *line, const char *want) {
    const char *sp = strchr(line, ' ');
    if (!sp) return 0;
    sp = strchr(sp + 1, ' ');
    return sp && strcmp(sp + 1, want) == 0;
}

/* ---- tests ---- */

TEST(writes_a_line_of_averages_every_tick) {
    int a[2], b[2];
    EXPECT_EQ(pipe(a), 0);
    EXPECT_EQ(pipe(b), 0);
    int efd = eventfd(0, EFD_CLOEXEC);
    put(a[1], "10\n20\n30\n");
    put(b[1], "100\n300\n");
    int fds[] = { a[0], b[0] };
    struct daq_config cfg = { fds, 2, efd, 10000000, "daq1.log" };   /* 10 ms period */
    struct script sc = { .efd = efd, .ms = 75 };
    pthread_t t;
    pthread_create(&t, NULL, run_script, &sc);
    struct daq_stats st;
    int rc = daq_run(&cfg, &st);
    pthread_join(t, NULL);
    EXPECT_EQ(rc, 0);
    printf("ticks=%lu overruns=%lu\n", st.ticks, st.overruns);
    EXPECT_TRUE(st.ticks >= 3 && st.ticks <= 12);   /* ~7 in 75 ms; the host isn't real-time */
    EXPECT_EQ(st.samples, 5);
    EXPECT_EQ(st.errors, 0);
    slurp("daq1.log");
    char line[128], want[128];
    EXPECT_EQ(nth_line("T ", 0, line, sizeof line), (int)st.ticks);   /* one T line per tick */
    EXPECT_STR_EQ(line, "T 1 20.0 200.0");
    nth_line("T ", (int)st.ticks - 1, line, sizeof line);
    snprintf(want, sizeof want, "T %lu 20.0 200.0", st.ticks);
    EXPECT_STR_EQ(line, want);
    EXPECT_EQ(nth_line("END", -1, line, sizeof line), 1);
    snprintf(want, sizeof want, "END ticks=%lu samples=5 errors=0", st.ticks);
    EXPECT_STR_EQ(line, want);
    close(a[0]); close(a[1]); close(b[0]); close(b[1]); close(efd);
}

TEST(the_ring_buffer_keeps_only_the_last_8_samples) {
    int a[2], b[2];
    EXPECT_EQ(pipe(a), 0);
    EXPECT_EQ(pipe(b), 0);
    int efd = eventfd(0, EFD_CLOEXEC);
    put(a[1], "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n");      /* window = 5..12 */
    int fds[] = { a[0], b[0] };                                  /* b never says anything */
    struct daq_config cfg = { fds, 2, efd, 10000000, "daq2.log" };
    struct script sc = { .efd = efd, .ms = 35 };
    pthread_t t;
    pthread_create(&t, NULL, run_script, &sc);
    struct daq_stats st;
    EXPECT_EQ(daq_run(&cfg, &st), 0);
    pthread_join(t, NULL);
    EXPECT_EQ(st.samples, 12);
    slurp("daq2.log");
    char line[128];
    EXPECT_TRUE(nth_line("T ", 0, line, sizeof line) >= 1);
    EXPECT_STR_EQ(line, "T 1 8.5 -");                            /* no samples yet → "-" */
    close(a[0]); close(a[1]); close(b[0]); close(b[1]); close(efd);
}

TEST(split_lines_and_garbage_are_handled) {
    int a[2];
    EXPECT_EQ(pipe(a), 0);
    int efd = eventfd(0, EFD_CLOEXEC);
    /* a good value, two bad lines, a line that's far too long, and half a number... */
    put(a[1], "4\nabc\n\n123456789012345678901234567890123456789\n12");
    int fds[] = { a[0] };
    struct daq_config cfg = { fds, 1, efd, 10000000, "daq3.log" };
    struct script sc = { .efd = efd, .ms = 30, .late_fd = a[1], .late = "3\n", .stop_ms = 40 };   /* ...completed later */
    pthread_t t;
    pthread_create(&t, NULL, run_script, &sc);
    struct daq_stats st;
    EXPECT_EQ(daq_run(&cfg, &st), 0);
    pthread_join(t, NULL);
    EXPECT_EQ(st.samples, 2);          /* 4 and 123 */
    EXPECT_EQ(st.errors, 3);           /* "abc", "", and the 39-digit line */
    slurp("daq3.log");
    char line[128];
    int n = nth_line("T ", -1, line, sizeof line);
    EXPECT_TRUE(n >= 2);
    EXPECT_TRUE(tick_avgs_are(line, "63.5"));   /* (4 + 123) / 2 on the last tick */
    close(a[0]); close(a[1]); close(efd);
}

TEST(shutdown_drains_the_sensors_and_exits_promptly) {
    int a[2];
    EXPECT_EQ(pipe(a), 0);
    int efd = eventfd(0, EFD_CLOEXEC);
    char text[512] = "";
    for (int i = 1; i <= 40; i++) {
        char v[8];
        snprintf(v, sizeof v, "%d\n", i);
        strcat(text, v);
    }
    put(a[1], text);                   /* 111 bytes: more than one read's worth */
    uint64_t one = 1;
    EXPECT_EQ(write(efd, &one, sizeof one), 8);   /* SIGTERM arrived before we even started */
    int fds[] = { a[0] };
    struct daq_config cfg = { fds, 1, efd, 200000000, "daq5.log" };   /* 200 ms period */
    struct daq_stats st;
    double t0 = now_ms(CLOCK_MONOTONIC);
    EXPECT_EQ(daq_run(&cfg, &st), 0);
    double ms = now_ms(CLOCK_MONOTONIC) - t0;
    EXPECT_TRUE(ms < 100.0);           /* no waiting for a tick */
    EXPECT_EQ(st.samples, 40);         /* nothing that was already in the pipe is lost */
    EXPECT_EQ(st.ticks, 0);
    slurp("daq5.log");
    EXPECT_STR_EQ(out_text, "END ticks=0 samples=40 errors=0\n");
    close(a[0]); close(a[1]); close(efd);
}

TEST(rejects_bad_configs_and_never_leaks_an_fd) {
    int a[2];
    EXPECT_EQ(pipe(a), 0);
    int efd = eventfd(0, EFD_CLOEXEC);
    int fds[] = { a[0], a[0], a[0], a[0], a[0] };
    struct daq_stats st;
    int before = open_fd_count();
    struct daq_config bad_period = { fds, 1, efd, 0, "daq6.log" };
    EXPECT_EQ(daq_run(&bad_period, &st), -EINVAL);
    struct daq_config no_sensors = { fds, 0, efd, 10000000, "daq6.log" };
    EXPECT_EQ(daq_run(&no_sensors, &st), -EINVAL);
    struct daq_config too_many = { fds, 5, efd, 10000000, "daq6.log" };
    EXPECT_EQ(daq_run(&too_many, &st), -EINVAL);
    struct daq_config bad_dir = { fds, 1, efd, 10000000, "no/such/dir/daq.log" };
    EXPECT_EQ(daq_run(&bad_dir, &st), -ENOENT);
    int closed[] = { 999 };
    struct daq_config bad_fd = { closed, 1, efd, 10000000, "daq6.log" };
    EXPECT_EQ(daq_run(&bad_fd, &st), -EBADF);     /* epoll_ctl fails: clean up what you opened */
    for (int i = 0; i < 20; i++) {
        uint64_t one = 1;
        if (write(efd, &one, sizeof one) < 0) perror("eventfd");
        struct daq_config ok = { fds, 1, efd, 10000000, "daq6.log" };
        EXPECT_EQ(daq_run(&ok, &st), 0);
    }
    EXPECT_EQ(open_fd_count(), before);
    EXPECT_NE(fcntl(a[0], F_GETFD), -1);          /* the caller's fds stay open */
    EXPECT_NE(fcntl(efd, F_GETFD), -1);
    close(a[0]); close(a[1]); close(efd);
}

TEST(an_unplugged_sensor_is_dropped_without_spinning) {
    int a[2], b[2];
    EXPECT_EQ(pipe(a), 0);
    EXPECT_EQ(pipe(b), 0);
    int efd = eventfd(0, EFD_CLOEXEC);
    put(a[1], "5\n7");                 /* "7" never gets its newline... */
    close(a[1]);                       /* ...because the sensor is unplugged: EOF */
    put(b[1], "1\n");
    int fds[] = { a[0], b[0] };
    struct daq_config cfg = { fds, 2, efd, 10000000, "daq4.log" };
    struct script sc = { .efd = efd, .ms = 60 };
    pthread_t t;
    double cpu0 = now_ms(CLOCK_PROCESS_CPUTIME_ID);
    pthread_create(&t, NULL, run_script, &sc);
    struct daq_stats st;
    EXPECT_EQ(daq_run(&cfg, &st), 0);
    pthread_join(t, NULL);
    double cpu = now_ms(CLOCK_PROCESS_CPUTIME_ID) - cpu0;
    EXPECT_EQ(st.samples, 2);
    EXPECT_EQ(st.errors, 1);           /* the unfinished "7" */
    EXPECT_TRUE(cpu < 30.0);           /* EOF stays readable forever: stop watching that fd */
    slurp("daq4.log");
    char line[128];
    EXPECT_TRUE(nth_line("T ", -1, line, sizeof line) >= 1);
    EXPECT_TRUE(tick_avgs_are(line, "5.0 1.0"));
    close(a[0]); close(b[0]); close(b[1]); close(efd);
}
