#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#define DAQ_MAX_SENSORS 4
#define DAQ_WINDOW 8      /* averages cover the last 8 samples of each sensor */
#define DAQ_LINE_MAX 32   /* a line of 32+ characters is garbage */

struct daq_config {
    const int *sensor_fds;
    size_t n_sensors;     /* 1..DAQ_MAX_SENSORS */
    int shutdown_fd;      /* eventfd: readable = shut down */
    int64_t period_ns;    /* sampling period, > 0 */
    const char *out_path;
};

struct daq_stats {
    unsigned long ticks;
    unsigned long samples;
    unsigned long errors;
    unsigned long overruns;
};

struct sensor {
    int fd;
    bool online;
    long ring[DAQ_WINDOW];   /* the last DAQ_WINDOW samples */
    size_t next, count;
    char line[DAQ_LINE_MAX];  /* the partial line received so far */
    size_t len;
    bool overlong;            /* discarding until the next '\n' */
};

struct daq {
    const struct daq_config *cfg;
    struct daq_stats *st;
    struct sensor sensors[DAQ_MAX_SENSORS];
    FILE *out;
    int epfd, tfd;
};

/* epoll data.u32 tags */
enum { TAG_TIMER = 100, TAG_SHUTDOWN = 101 };

static void finish_line(struct daq *d, struct sensor *s) {
    if (s->overlong) {
        d->st->errors++;
    } else {
        s->line[s->len] = '\0';
        char *end;
        errno = 0;
        long v = strtol(s->line, &end, 10);
        if (end == s->line || *end != '\0' || errno == ERANGE) {
            d->st->errors++;
        } else {
            s->ring[s->next] = v;
            s->next = (s->next + 1) % DAQ_WINDOW;
            if (s->count < DAQ_WINDOW) s->count++;
            d->st->samples++;
        }
    }
    s->len = 0;
    s->overlong = false;
}

static void on_sensor(struct daq *d, struct sensor *s) {
    char buf[64];
    ssize_t n = read(s->fd, buf, sizeof buf);
    if (n < 0 && (errno == EINTR || errno == EAGAIN)) return;
    if (n <= 0) {                                  /* EOF or a read error: the sensor is gone */
        if (s->len > 0 || s->overlong) d->st->errors++;   /* an unfinished line */
        epoll_ctl(d->epfd, EPOLL_CTL_DEL, s->fd, NULL);
        s->online = false;
        return;
    }
    for (ssize_t i = 0; i < n; i++) {              /* lines can be split across reads */
        if (buf[i] == '\n') finish_line(d, s);
        else if (s->len < DAQ_LINE_MAX - 1) s->line[s->len++] = buf[i];
        else s->overlong = true;
    }
}

static void on_tick(struct daq *d) {
    uint64_t expired;
    if (read(d->tfd, &expired, sizeof expired) != (ssize_t)sizeof expired) return;
    d->st->ticks++;
    d->st->overruns += expired - 1;
    fprintf(d->out, "T %lu", d->st->ticks);
    for (size_t i = 0; i < d->cfg->n_sensors; i++) {
        const struct sensor *s = &d->sensors[i];
        if (s->count == 0) {
            fputs(" -", d->out);
            continue;
        }
        long sum = 0;
        for (size_t k = 0; k < s->count; k++) sum += s->ring[k];
        fprintf(d->out, " %.1f", (double)sum / (double)s->count);
    }
    fputc('\n', d->out);
}

static int setup(struct daq *d) {
    const struct daq_config *cfg = d->cfg;
    d->out = fopen(cfg->out_path, "w");
    if (!d->out) return -errno;
    if ((d->epfd = epoll_create1(EPOLL_CLOEXEC)) < 0) return -errno;
    if ((d->tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC)) < 0) return -errno;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    int64_t first = (int64_t)now.tv_nsec + cfg->period_ns;
    struct itimerspec spec = {
        .it_value = { .tv_sec = now.tv_sec + (time_t)(first / 1000000000), .tv_nsec = (long)(first % 1000000000) },
        .it_interval = { .tv_sec = (time_t)(cfg->period_ns / 1000000000), .tv_nsec = (long)(cfg->period_ns % 1000000000) },
    };
    if (timerfd_settime(d->tfd, TFD_TIMER_ABSTIME, &spec, NULL) < 0) return -errno;

    struct epoll_event ev = { .events = EPOLLIN, .data.u32 = TAG_TIMER };
    if (epoll_ctl(d->epfd, EPOLL_CTL_ADD, d->tfd, &ev) < 0) return -errno;
    ev.data.u32 = TAG_SHUTDOWN;
    if (epoll_ctl(d->epfd, EPOLL_CTL_ADD, cfg->shutdown_fd, &ev) < 0) return -errno;
    for (size_t i = 0; i < cfg->n_sensors; i++) {
        d->sensors[i] = (struct sensor){ .fd = cfg->sensor_fds[i], .online = true };
        ev.data.u32 = (uint32_t)i;
        if (epoll_ctl(d->epfd, EPOLL_CTL_ADD, d->sensors[i].fd, &ev) < 0) return -errno;
    }
    return 0;
}

static int loop(struct daq *d) {
    struct epoll_event events[DAQ_MAX_SENSORS + 2];
    bool stopping = false;
    for (;;) {
        int n = epoll_wait(d->epfd, events, DAQ_MAX_SENSORS + 2, stopping ? 0 : -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -errno;
        }
        if (n == 0 && stopping) return 0;          /* drained: nothing left to read */
        for (int i = 0; i < n; i++) {
            uint32_t tag = events[i].data.u32;
            if (tag == TAG_TIMER) {
                on_tick(d);
            } else if (tag == TAG_SHUTDOWN) {
                uint64_t v;
                if (read(d->cfg->shutdown_fd, &v, sizeof v) < 0) { /* already reset */ }
                stopping = true;
                epoll_ctl(d->epfd, EPOLL_CTL_DEL, d->cfg->shutdown_fd, NULL);
            } else if (d->sensors[tag].online) {
                on_sensor(d, &d->sensors[tag]);
            }
        }
    }
}

int daq_run(const struct daq_config *cfg, struct daq_stats *st) {
    if (!cfg || !st || !cfg->out_path || !cfg->sensor_fds || cfg->n_sensors == 0 ||
        cfg->n_sensors > DAQ_MAX_SENSORS || cfg->period_ns <= 0)
        return -EINVAL;
    *st = (struct daq_stats){0};
    struct daq d = { .cfg = cfg, .st = st, .epfd = -1, .tfd = -1 };

    int rc = setup(&d);
    if (rc == 0) rc = loop(&d);
    if (rc == 0) fprintf(d.out, "END ticks=%lu samples=%lu errors=%lu\n", st->ticks, st->samples, st->errors);

    if (d.tfd >= 0) close(d.tfd);
    if (d.epfd >= 0) close(d.epfd);
    if (d.out) fclose(d.out);
    return rc;
}
