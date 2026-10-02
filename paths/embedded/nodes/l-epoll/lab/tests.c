#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* A fake sensor: the handler appends whatever it reads, `chunk` bytes at a time. */
struct sensor {
    char got[256];
    size_t len;
    int calls;
    size_t chunk;
};

static bool on_sensor(int fd, void *ctx) {
    struct sensor *s = ctx;
    s->calls++;
    size_t want = s->chunk ? s->chunk : 64;
    if (want > sizeof s->got - 1 - s->len) want = sizeof s->got - 1 - s->len;
    ssize_t n = read(fd, s->got + s->len, want);
    if (n <= 0) return false;            /* EOF or error: stop watching this sensor */
    s->len += (size_t)n;
    s->got[s->len] = '\0';
    return true;
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

static void sleep_ms(long ms) {
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&ts, NULL);
}

static double cpu_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    return (double)ts.tv_sec * 1e3 + (double)ts.tv_nsec / 1e6;
}

TEST(dispatches_each_fd_to_its_own_handler) {
    int p[3][2];
    struct sensor s[3];
    memset(s, 0, sizeof s);
    struct evloop *loop = evloop_create();
    EXPECT_NOT_NULL(loop);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(pipe(p[i]), 0);
        EXPECT_EQ(evloop_add(loop, p[i][0], on_sensor, &s[i]), 0);
    }
    put(p[0][1], "temp=41");
    put(p[1][1], "hum=55");
    put(p[2][1], "press=1013");
    EXPECT_EQ(evloop_request_stop(loop), 0);
    EXPECT_EQ(evloop_run(loop), 3);          /* one call per readable sensor */
    EXPECT_STR_EQ(s[0].got, "temp=41");
    EXPECT_STR_EQ(s[1].got, "hum=55");
    EXPECT_STR_EQ(s[2].got, "press=1013");
    evloop_destroy(loop);
    for (int i = 0; i < 3; i++) { close(p[i][0]); close(p[i][1]); }
}

TEST(drains_pending_data_before_shutting_down) {
    int p[2];
    EXPECT_EQ(pipe(p), 0);
    struct sensor s = { .chunk = 4 };        /* a slow handler: 4 bytes per call */
    struct evloop *loop = evloop_create();
    EXPECT_NOT_NULL(loop);
    EXPECT_EQ(evloop_add(loop, p[0], on_sensor, &s), 0);
    put(p[1], "0123456789abcdefghijklmnopqrstuvwxyzABCD");   /* 40 bytes */
    evloop_request_stop(loop);
    evloop_request_stop(loop);               /* asking twice is fine */
    EXPECT_EQ(evloop_run(loop), 10);         /* level-triggered: called again while data remains */
    EXPECT_EQ(s.len, 40);
    evloop_destroy(loop);
    close(p[0]);
    close(p[1]);
}

struct later { int fd; struct evloop *loop; };

static void *late_events(void *arg) {
    struct later *l = arg;
    sleep_ms(30);
    put(l->fd, "hello");
    sleep_ms(30);
    evloop_request_stop(l->loop);
    return NULL;
}

TEST(sleeps_until_events_arrive_from_another_thread) {
    int p[2];
    EXPECT_EQ(pipe(p), 0);
    struct sensor s = {0};
    struct evloop *loop = evloop_create();
    EXPECT_NOT_NULL(loop);
    EXPECT_EQ(evloop_add(loop, p[0], on_sensor, &s), 0);
    struct later l = { p[1], loop };
    pthread_t t;
    double cpu0 = cpu_ms();
    pthread_create(&t, NULL, late_events, &l);
    int calls = evloop_run(loop);
    pthread_join(t, NULL);
    double cpu = cpu_ms() - cpu0;
    EXPECT_EQ(calls, 1);
    EXPECT_STR_EQ(s.got, "hello");
    /* ~60 ms of waiting should cost almost no CPU. A busy-polling loop burns all of it. */
    EXPECT_TRUE(cpu < 30.0);
    evloop_destroy(loop);
    close(p[0]);
    close(p[1]);
}

TEST(add_reports_errors_as_negative_errno) {
    struct evloop *loop = evloop_create();
    EXPECT_NOT_NULL(loop);
    struct sensor s = {0};
    EXPECT_EQ(evloop_add(loop, 987, on_sensor, &s), -EBADF);       /* not an open fd */
    int file = open("plain.txt", O_RDWR | O_CREAT, 0644);
    EXPECT_EQ(evloop_add(loop, file, on_sensor, &s), -EPERM);      /* epoll can't watch regular files */
    close(file);
    int p[17][2];
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(pipe(p[i]), 0);
        EXPECT_EQ(evloop_add(loop, p[i][0], on_sensor, &s), 0);
    }
    EXPECT_EQ(pipe(p[16]), 0);
    EXPECT_EQ(evloop_add(loop, p[16][0], on_sensor, &s), -ENOSPC);
    evloop_destroy(loop);
    for (int i = 0; i < 17; i++) { close(p[i][0]); close(p[i][1]); }
}

TEST(destroy_closes_its_own_fds_but_not_yours) {
    int p[2];
    EXPECT_EQ(pipe(p), 0);
    struct sensor s = {0};
    int before = open_fd_count();
    for (int i = 0; i < 100; i++) {
        struct evloop *loop = evloop_create();
        EXPECT_NOT_NULL(loop);
        EXPECT_EQ(evloop_add(loop, p[0], on_sensor, &s), 0);
        evloop_request_stop(loop);
        evloop_run(loop);
        evloop_destroy(loop);
    }
    EXPECT_EQ(open_fd_count(), before);
    EXPECT_NE(fcntl(p[0], F_GETFD), -1);     /* the caller's pipe is still open */
    close(p[0]);
    close(p[1]);
}

TEST(a_handler_returning_false_is_removed) {
    int p[2];
    EXPECT_EQ(pipe(p), 0);
    struct sensor s = {0};
    struct evloop *loop = evloop_create();
    EXPECT_NOT_NULL(loop);
    EXPECT_EQ(evloop_add(loop, p[0], on_sensor, &s), 0);
    put(p[1], "bye");
    close(p[1]);                             /* sensor unplugged: EOF stays "ready" forever */
    evloop_request_stop(loop);
    EXPECT_EQ(evloop_run(loop), 2);          /* "bye", then EOF → handler returns false */
    EXPECT_STR_EQ(s.got, "bye");
    EXPECT_EQ(s.calls, 2);
    evloop_destroy(loop);
    close(p[0]);
}
