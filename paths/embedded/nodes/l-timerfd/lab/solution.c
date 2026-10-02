#include <errno.h>
#include <stdint.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#define NSEC_PER_SEC 1000000000LL

/* t + ns, normalised so that 0 <= tv_nsec < 1e9 (ns may be negative or several seconds). */
struct timespec ts_add_ns(struct timespec t, int64_t ns) {
    int64_t sec = (int64_t)t.tv_sec + ns / NSEC_PER_SEC;
    int64_t nsec = (int64_t)t.tv_nsec + ns % NSEC_PER_SEC;   /* in (-2e9, 2e9) */
    if (nsec >= NSEC_PER_SEC) {
        nsec -= NSEC_PER_SEC;
        sec++;
    } else if (nsec < 0) {
        nsec += NSEC_PER_SEC;
        sec--;
    }
    return (struct timespec){ .tv_sec = (time_t)sec, .tv_nsec = (long)nsec };
}

/* a - b in nanoseconds. */
int64_t ts_diff_ns(struct timespec a, struct timespec b) {
    return ((int64_t)a.tv_sec - (int64_t)b.tv_sec) * NSEC_PER_SEC + ((int64_t)a.tv_nsec - (int64_t)b.tv_nsec);
}

struct periodic_stats {
    unsigned cycles;        /* times fn was called */
    uint64_t expirations;   /* timer periods that elapsed (sum of the timerfd counts) */
    uint64_t overruns;      /* periods we missed: expirations - cycles */
};

typedef void (*periodic_fn)(unsigned cycle, void *ctx);

int run_periodic(int64_t period_ns, unsigned cycles, periodic_fn fn, void *ctx, struct periodic_stats *st) {
    if (period_ns <= 0 || cycles == 0 || !fn || !st) return -EINVAL;
    *st = (struct periodic_stats){0};

    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (tfd < 0) return -errno;

    /* An absolute grid: start + k * period. Work time can never push it later. */
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    struct itimerspec spec = {
        .it_value = ts_add_ns(now, period_ns),
        .it_interval = ts_add_ns((struct timespec){0}, period_ns),
    };
    int rc = 0;
    if (timerfd_settime(tfd, TFD_TIMER_ABSTIME, &spec, NULL) < 0) rc = -errno;

    while (rc == 0 && st->cycles < cycles) {
        uint64_t expired;
        ssize_t n = read(tfd, &expired, sizeof expired);   /* blocks until the next period */
        if (n < 0 && errno == EINTR) continue;
        if (n != (ssize_t)sizeof expired) {
            rc = n < 0 ? -errno : -EIO;
            break;
        }
        st->expirations += expired;
        st->overruns += expired - 1;                       /* more than 1 = we were late */
        fn(st->cycles, ctx);
        st->cycles++;
    }
    close(tfd);
    return rc;
}
