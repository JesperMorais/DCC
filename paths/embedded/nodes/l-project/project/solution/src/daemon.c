#include "daemon.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#include "channel.h"
#include "config.h"
#include "control.h"
#include "csvlog.h"
#include "stream.h"
#include "util.h"

/* epoll_event.data.u64 says which fd woke us. Clients are TAG_CLIENT + slot. */
enum { TAG_SIGNAL = 1, TAG_TIMER, TAG_FIFO, TAG_LISTEN, TAG_CLIENT = 100 };

struct sensord {
    const char *config_path;
    struct config cfg;
    int ep, sig_fd, timer_fd, fifo_fd;
    struct stream stream;
    struct csvlog log;
    struct control ctl;
    long tick;
    unsigned long temp_errors;
    bool stopping;
};

static int watch(struct sensord *d, int fd, uint64_t tag) {
    struct epoll_event ev = { .events = EPOLLIN, .data.u64 = tag };
    return epoll_ctl(d->ep, EPOLL_CTL_ADD, fd, &ev);
}

/* ---- the sensor FIFO ---- */

/* Opens the FIFO for reading, creating it if needed. Returns the fd or -1. */
static int open_fifo(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        if (errno != ENOENT || mkfifo(path, 0660) != 0) return -1;
    } else if (!S_ISFIFO(st.st_mode)) {
        errno = EEXIST;
        return -1;
    }
    /* O_NONBLOCK: don't wait in open() for a writer, and never block in read(). */
    return open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
}

/* Reads whatever is in the FIFO. Returns false at EOF (the writer left). */
static bool read_fifo(struct sensord *d) {
    char buf[4096];
    for (;;) {
        ssize_t n = read(d->fifo_fd, buf, sizeof buf);
        if (n > 0) stream_feed(&d->stream, buf, (size_t)n);
        else if (n < 0 && errno == EINTR) continue;
        else if (n < 0 && errno == EAGAIN) return true;
        else return false;
    }
}

static int on_fifo(struct sensord *d) {
    if (read_fifo(d)) return 0;
    /* EOF stays readable forever, so swap in a fresh read end. Open the new one
     * before closing the old one: with no reader at all, a writer that shows
     * up in between would get EPIPE. A new read end doesn't report EOF until
     * a writer has come and gone again. */
    stream_finish(&d->stream);
    int fd = open_fifo(d->cfg.fifo);
    if (fd < 0 || watch(d, fd, TAG_FIFO) != 0) {
        fprintf(stderr, "sensord: %s: %s\n", d->cfg.fifo, strerror(errno));
        if (fd >= 0) close(fd);
        return -1;
    }
    epoll_ctl(d->ep, EPOLL_CTL_DEL, d->fifo_fd, NULL);
    close(d->fifo_fd);
    d->fifo_fd = fd;
    return 0;
}

/* ---- the periodic tick ---- */

static int arm_timer(struct sensord *d) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    struct timespec period = { d->cfg.period_ms / 1000, (long)(d->cfg.period_ms % 1000) * 1000000L };
    struct itimerspec its = { .it_interval = period, .it_value = now };
    its.it_value.tv_sec += period.tv_sec;
    its.it_value.tv_nsec += period.tv_nsec;
    if (its.it_value.tv_nsec >= 1000000000L) {
        its.it_value.tv_sec++;
        its.it_value.tv_nsec -= 1000000000L;
    }
    /* An absolute first expiry plus an interval: the kernel keeps the grid. */
    return timerfd_settime(d->timer_fd, TFD_TIMER_ABSTIME, &its, NULL);
}

static int write_row(struct sensord *d) {
    char temp[32] = "", min[32] = "", mean[32] = "", max[32] = "", row[192];
    double t;
    if (channel_read_temp(d->cfg.sysfs, &t) == 0) snprintf(temp, sizeof temp, "%.3f", t);
    else d->temp_errors++;
    const struct window *w = &d->stream.win;
    if (w->count > 0) {
        snprintf(min, sizeof min, "%.2f", w->min);
        snprintf(mean, sizeof mean, "%.2f", w->sum / w->count);
        snprintf(max, sizeof max, "%.2f", w->max);
    }
    snprintf(row, sizeof row, "%ld,%s,%d,%s,%s,%s", d->tick, temp, w->count, min, mean, max);
    window_reset(&d->stream.win);
    if (csvlog_write(&d->log, row) != 0) {
        fprintf(stderr, "sensord: %s: %s\n", d->log.path, strerror(errno));
        return -1;
    }
    return 0;
}

static int on_timer(struct sensord *d) {
    uint64_t expirations;
    if (read(d->timer_fd, &expirations, sizeof expirations) != sizeof expirations) return 0;
    /* More than one expiration means we missed periods: the tick jumps. */
    d->tick += (long)expirations;
    return write_row(d);
}

/* ---- signals ---- */

static void reload(struct sensord *d) {
    struct config next;
    char err[512];
    if (config_load(d->config_path, &next, err, sizeof err) != 0) {
        fprintf(stderr, "sensord: reload failed, keeping the old config: %s\n", err);
        return;
    }
    const char *keys[] = { "fifo", "log_dir", "control" };
    char *olds[] = { d->cfg.fifo, d->cfg.log_dir, d->cfg.control };
    char *news[] = { next.fifo, next.log_dir, next.control };
    for (int i = 0; i < 3; i++) {
        if (strcmp(olds[i], news[i]) != 0) {
            fprintf(stderr, "sensord: reload: changing %s needs a restart, keeping %s\n", keys[i], olds[i]);
            strcpy(news[i], olds[i]);
        }
    }
    bool new_period = next.period_ms != d->cfg.period_ms;
    d->cfg = next;
    d->log.rotate_lines = next.rotate_lines;
    d->log.keep = next.keep;
    if (new_period) arm_timer(d);
    fprintf(stderr, "sensord: reloaded %s\n", d->config_path);
}

static void on_signal(struct sensord *d) {
    struct signalfd_siginfo si;
    while (read(d->sig_fd, &si, sizeof si) == sizeof si) {
        if (si.ssi_signo == SIGHUP) reload(d);
        else d->stopping = true; /* SIGTERM or SIGINT */
    }
}

/* ---- the control socket ---- */

static void command(struct sensord *d, int slot, char *line) {
    char reply[192];
    char *cmd = trim(line);
    if (strcmp(cmd, "ping") == 0) snprintf(reply, sizeof reply, "pong\n");
    else if (strcmp(cmd, "stats") == 0)
        snprintf(reply, sizeof reply, "ticks=%ld samples=%lu errors=%lu rotations=%lu\n", d->tick,
                 d->stream.samples, d->stream.errors + d->temp_errors, d->log.rotations);
    else snprintf(reply, sizeof reply, "error unknown command \"%.32s\"\n", cmd);
    /* Replies are tiny and the socket buffer is big. A client that doesn't
     * read them for long enough to fill it gets dropped. */
    if (write(d->ctl.clients[slot].fd, reply, strlen(reply)) != (ssize_t)strlen(reply)) {
        epoll_ctl(d->ep, EPOLL_CTL_DEL, d->ctl.clients[slot].fd, NULL);
        control_drop(&d->ctl, slot);
    }
}

static void on_client(struct sensord *d, int slot) {
    struct client *c = &d->ctl.clients[slot];
    ssize_t n = read(c->fd, c->buf + c->len, sizeof c->buf - c->len);
    if (n < 0 && (errno == EAGAIN || errno == EINTR)) return;
    if (n <= 0) {
        epoll_ctl(d->ep, EPOLL_CTL_DEL, c->fd, NULL);
        control_drop(&d->ctl, slot);
        return;
    }
    c->len += (size_t)n;
    char *nl;
    while (c->fd >= 0 && (nl = memchr(c->buf, '\n', c->len))) {
        *nl = '\0';
        command(d, slot, c->buf);
        size_t used = (size_t)(nl - c->buf) + 1;
        memmove(c->buf, nl + 1, c->len - used);
        c->len -= used;
    }
    if (c->fd >= 0 && c->len == sizeof c->buf) { /* a "line" that never ends */
        epoll_ctl(d->ep, EPOLL_CTL_DEL, c->fd, NULL);
        control_drop(&d->ctl, slot);
    }
}

static void on_listen(struct sensord *d) {
    int slot = control_accept(&d->ctl);
    if (slot >= 0 && watch(d, d->ctl.clients[slot].fd, TAG_CLIENT + (uint64_t)slot) != 0) control_drop(&d->ctl, slot);
}

/* ---- setup, loop, shutdown ---- */

static int setup(struct sensord *d) {
    char err[512];
    if (config_load(d->config_path, &d->cfg, err, sizeof err) != 0) {
        fprintf(stderr, "sensord: %s\n", err);
        return -1;
    }
    /* Block the signals before anything else, so they queue up for the
     * signalfd instead of killing us halfway through setup. */
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGTERM);
    sigaddset(&sigs, SIGINT);
    sigaddset(&sigs, SIGHUP);
    sigprocmask(SIG_BLOCK, &sigs, NULL);
    signal(SIGPIPE, SIG_IGN); /* a client that hangs up mid-reply is not fatal */

    const char *what = "epoll";
    if ((d->ep = epoll_create1(EPOLL_CLOEXEC)) < 0) goto fail;
    if ((d->sig_fd = signalfd(-1, &sigs, SFD_NONBLOCK | SFD_CLOEXEC)) < 0) goto fail;
    what = d->cfg.fifo;
    if ((d->fifo_fd = open_fifo(d->cfg.fifo)) < 0) goto fail;
    what = d->cfg.log_dir;
    if (csvlog_open(&d->log, d->cfg.log_dir, d->cfg.rotate_lines, d->cfg.keep) != 0) goto fail;
    what = d->cfg.control;
    if (*d->cfg.control && control_open(&d->ctl, d->cfg.control) != 0) goto fail;
    what = "timerfd";
    if ((d->timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)) < 0) goto fail;
    if (arm_timer(d) != 0) goto fail;
    what = "epoll_ctl";
    if (watch(d, d->sig_fd, TAG_SIGNAL) || watch(d, d->timer_fd, TAG_TIMER) || watch(d, d->fifo_fd, TAG_FIFO))
        goto fail;
    if (d->ctl.listen_fd >= 0 && watch(d, d->ctl.listen_fd, TAG_LISTEN)) goto fail;
    return 0;
fail:
    fprintf(stderr, "sensord: %s: %s\n", what, strerror(errno));
    return -1;
}

static int loop(struct sensord *d) {
    struct epoll_event ready[16];
    while (!d->stopping) {
        int n = epoll_wait(d->ep, ready, 16, -1);
        if (n < 0 && errno == EINTR) continue;
        if (n < 0) return -1;
        for (int i = 0; i < n; i++) {
            uint64_t tag = ready[i].data.u64;
            int rc = 0;
            if (tag == TAG_SIGNAL) on_signal(d);
            else if (tag == TAG_TIMER) rc = on_timer(d);
            else if (tag == TAG_FIFO) rc = on_fifo(d);
            else if (tag == TAG_LISTEN) on_listen(d);
            else if (d->ctl.clients[tag - TAG_CLIENT].fd >= 0) on_client(d, (int)(tag - TAG_CLIENT));
            if (rc) return -1;
        }
    }
    /* Drain: bytes already in the pipe belong in the log. Then the final row
     * for the unfinished period. */
    read_fifo(d);
    stream_finish(&d->stream);
    d->tick++;
    return write_row(d);
}

int daemon_run(const char *config_path) {
    struct sensord d = { .config_path = config_path, .ep = -1, .sig_fd = -1, .timer_fd = -1, .fifo_fd = -1 };
    d.log.fd = -1;
    control_init(&d.ctl);
    int rc = setup(&d) == 0 && loop(&d) == 0 ? 0 : 1;
    /* One exit path: close what we opened, whichever step failed. */
    control_close(&d.ctl);
    csvlog_close(&d.log);
    if (d.timer_fd >= 0) close(d.timer_fd);
    if (d.fifo_fd >= 0) close(d.fifo_fd);
    if (d.sig_fd >= 0) close(d.sig_fd);
    if (d.ep >= 0) close(d.ep);
    return rc;
}
