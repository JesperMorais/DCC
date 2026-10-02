#include <errno.h>
#include <stdint.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#define NSEC_PER_SEC 1000000000LL

/* t + ns, normalised so that 0 <= tv_nsec < 1e9 (ns may be negative or several seconds). */
struct timespec ts_add_ns(struct timespec t, int64_t ns) {
    t.tv_nsec += (long)ns;   /* TODO: carry into tv_sec */
    return t;
}

/* a - b in nanoseconds. */
int64_t ts_diff_ns(struct timespec a, struct timespec b) {
    return (int64_t)(a.tv_nsec - b.tv_nsec);   /* TODO: seconds too */
}

struct periodic_stats {
    unsigned cycles;        /* times fn was called */
    uint64_t expirations;   /* timer periods that elapsed (sum of the timerfd counts) */
    uint64_t overruns;      /* periods we missed: expirations - cycles */
};

typedef void (*periodic_fn)(unsigned cycle, void *ctx);

int run_periodic(int64_t period_ns, unsigned cycles, periodic_fn fn, void *ctx, struct periodic_stats *st) {
    /* A first attempt: sleep one period after each piece of work. What's wrong with it? */
    if (period_ns <= 0 || cycles == 0 || !fn || !st) return -EINVAL;
    *st = (struct periodic_stats){0};
    struct timespec delay = { 0, (long)period_ns };
    for (unsigned i = 0; i < cycles; i++) {
        nanosleep(&delay, NULL);
        fn(i, ctx);
        st->cycles++;
        st->expirations++;
    }
    return 0;
}
