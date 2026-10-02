#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

static struct timespec ts(long long sec, long nsec) { return (struct timespec){ .tv_sec = (time_t)sec, .tv_nsec = nsec }; }

static int64_t to_ns(struct timespec t) { return (int64_t)t.tv_sec * 1000000000LL + t.tv_nsec; }

static struct timespec mono_now(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t;
}

static double ms_since(struct timespec start) {
    struct timespec now = mono_now();
    return (double)((now.tv_sec - start.tv_sec) * 1000000000LL + (now.tv_nsec - start.tv_nsec)) / 1e6;
}

static void sleep_ms(long ms) {
    struct timespec t = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&t, NULL);
}

static int open_fd_count(void) {
    int n = 0;
    DIR *d = opendir("/proc/self/fd");
    if (!d) return -1;
    while (readdir(d)) n++;
    closedir(d);
    return n;
}

/* ---- the pure helpers ---- */

TEST(ts_add_ns_carries_into_seconds) {
    struct timespec r = ts_add_ns(ts(10, 999999999), 1);
    EXPECT_EQ(r.tv_sec, 11);
    EXPECT_EQ(r.tv_nsec, 0);
    r = ts_add_ns(ts(10, 600000000), 500000000);          /* 0.6 s + 0.5 s */
    EXPECT_EQ(r.tv_sec, 11);
    EXPECT_EQ(r.tv_nsec, 100000000);
    r = ts_add_ns(ts(10, 0), 2500000000LL);               /* periods longer than a second */
    EXPECT_EQ(r.tv_sec, 12);
    EXPECT_EQ(r.tv_nsec, 500000000);
    r = ts_add_ns(ts(10, 100), -200);                     /* borrowing backwards */
    EXPECT_EQ(r.tv_sec, 9);
    EXPECT_EQ(r.tv_nsec, 999999900);
}

TEST(ts_add_ns_is_exact_for_every_combination) {
    const long long secs[] = { 0, 1, 7, 1700000000LL };
    const long nsecs[] = { 0, 1, 2, 499999999, 500000000, 500000001, 999999998, 999999999 };
    const int64_t adds[] = { 0, 1, -1, 999999999, -999999999, 1000000000, -1000000000, 1000000001,
                             -1000000001, 1999999999, 2000000000, 5000000007LL, -3000000001LL, 4999 };
    int checked = 0;
    for (size_t a = 0; a < sizeof secs / sizeof *secs; a++)
        for (size_t b = 0; b < sizeof nsecs / sizeof *nsecs; b++)
            for (size_t c = 0; c < sizeof adds / sizeof *adds; c++) {
                struct timespec t = ts(secs[a], nsecs[b]);
                struct timespec r = ts_add_ns(t, adds[c]);
                EXPECT_TRUE(r.tv_nsec >= 0 && r.tv_nsec < 1000000000L);   /* normalised */
                EXPECT_EQ(to_ns(r), to_ns(t) + adds[c]);                  /* and exact */
                checked++;
            }
    EXPECT_EQ(checked, 448);
}

TEST(ts_diff_ns_handles_the_borrow) {
    EXPECT_EQ(ts_diff_ns(ts(5, 0), ts(4, 999999999)), 1);
    EXPECT_EQ(ts_diff_ns(ts(4, 999999999), ts(5, 0)), -1);
    EXPECT_EQ(ts_diff_ns(ts(1700000003LL, 250000000), ts(1700000000LL, 750000000)), 2500000000LL);
    EXPECT_EQ(ts_diff_ns(ts(42, 42), ts(42, 42)), 0);
}

/* ---- the real timerfd loop (generous tolerances: the host isn't real-time) ---- */

struct trace {
    struct timespec at[64];
    unsigned n;
    unsigned slow_cycle;   /* this cycle overruns on purpose... */
    long slow_ms;          /* ...by sleeping this long */
    long work_ms;          /* every cycle "works" this long */
};

static void work(unsigned cycle, void *ctx) {
    struct trace *t = ctx;
    if (t->n < 64) t->at[t->n++] = mono_now();
    if (t->work_ms) sleep_ms(t->work_ms);
    if (t->slow_ms && cycle == t->slow_cycle) sleep_ms(t->slow_ms);
}

TEST(runs_ten_cycles_of_five_ms_and_closes_the_timerfd) {
    struct trace t = {0};
    struct periodic_stats st;
    int fds = open_fd_count();
    struct timespec start = mono_now();
    EXPECT_EQ(run_periodic(5000000, 10, work, &t, &st), 0);
    double ms = ms_since(start);
    EXPECT_EQ(st.cycles, 10);
    EXPECT_EQ(t.n, 10);
    EXPECT_EQ(st.expirations, st.cycles + st.overruns);
    EXPECT_TRUE(ms >= 49.0);           /* a timer never fires early */
    EXPECT_TRUE(ms < 500.0);
    EXPECT_EQ(open_fd_count(), fds);   /* the timerfd was closed */
    EXPECT_EQ(run_periodic(0, 10, work, &t, &st), -EINVAL);
    EXPECT_EQ(run_periodic(5000000, 0, work, &t, &st), -EINVAL);
}

TEST(a_slow_cycle_shows_up_as_overruns) {
    struct trace t = { .slow_cycle = 2, .slow_ms = 23 };   /* > 4 periods of 5 ms */
    struct periodic_stats st;
    EXPECT_EQ(run_periodic(5000000, 6, work, &t, &st), 0);
    EXPECT_EQ(st.cycles, 6);
    printf("expirations=%llu overruns=%llu\n", (unsigned long long)st.expirations, (unsigned long long)st.overruns);
    EXPECT_TRUE(st.overruns >= 3);     /* the next read() reports >= 4 expirations at once */
    EXPECT_EQ(st.expirations, st.cycles + st.overruns);
}

TEST(work_time_does_not_make_the_schedule_drift) {
    struct trace t = { .work_ms = 6 };                     /* 6 ms of work in a 10 ms period */
    struct periodic_stats st;
    EXPECT_EQ(run_periodic(10000000, 8, work, &t, &st), 0);
    EXPECT_EQ(t.n, 8);
    /* Absolute deadlines: cycle 7 starts ~70 ms after cycle 0. A "work, then sleep 10 ms"
     * loop starts it ~112 ms after (7 × 16 ms): the work time piles up as drift. */
    double span = (double)ts_diff_ns(t.at[7], t.at[0]) / 1e6;
    printf("cycle 0 -> cycle 7: %.1f ms (ideal 70.0)\n", span);
    EXPECT_TRUE(span >= 60.0);
    EXPECT_TRUE(span < 95.0);
}
