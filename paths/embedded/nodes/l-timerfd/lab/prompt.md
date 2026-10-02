A motor controller must run its control law every 5 ms. Not "5 ms after it last finished", but at 5, 10, 15… ms, on a fixed grid. If one cycle runs late, you need to **know** about it, because a missed period is a real-time bug you'll want in the logs.

The starter does the obvious thing (work, then `nanosleep(period)`) and drifts. Fix it in two parts.

**Part 1: pure `timespec` arithmetic.** It has no syscalls, so the tests can check it exhaustively.

```c
struct timespec ts_add_ns(struct timespec t, int64_t ns);   /* t + ns */
int64_t ts_diff_ns(struct timespec a, struct timespec b);   /* a - b, in ns */
```

- The result of `ts_add_ns` is **normalised**: `0 <= tv_nsec < 1 000 000 000`.
- `ns` can be negative, and it can be bigger than a second (`2500000000` is 2.5 s).

```c
ts_add_ns({10, 999999999}, 1)          // → {11, 0}
ts_add_ns({10, 0}, 2500000000)         // → {12, 500000000}
ts_add_ns({10, 100}, -200)             // → {9, 999999900}
ts_diff_ns({5, 0}, {4, 999999999})     // → 1
```

**Part 2: a real periodic loop on a `timerfd`.**

```c
struct periodic_stats { unsigned cycles; uint64_t expirations; uint64_t overruns; };
typedef void (*periodic_fn)(unsigned cycle, void *ctx);

int run_periodic(int64_t period_ns, unsigned cycles, periodic_fn fn, void *ctx, struct periodic_stats *st);
```

- Create a `timerfd` on `CLOCK_MONOTONIC`. The first expiry is at the absolute time **now + period** (`TFD_TIMER_ABSTIME`), and after that it fires every period.
- For each of the `cycles` cycles: `read()` the timerfd (it blocks until the next tick), add the 8-byte expiration count to `expirations`, add `count - 1` to `overruns`, then call `fn(cycle, ctx)` with cycle = 0, 1, 2…
- When the loop is done, `cycles` equals the number of `fn` calls, and `expirations == cycles + overruns`.
- Close the timerfd before returning. Return 0, `-EINVAL` if `period_ns <= 0` or `cycles == 0`, or `-errno` if a call fails.

What the tests check, with generous tolerances because your machine isn't real-time:
- 10 cycles of 5 ms take at least 50 ms.
- A cycle that oversleeps by 23 ms produces **at least 3 overruns**.
- With 6 ms of work per 10 ms period, cycle 7 starts about **70 ms** after cycle 0, not 112 ms.
