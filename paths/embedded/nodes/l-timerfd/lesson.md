A team porting a 1 kHz servo loop from an MCU to embedded Linux wrote the obvious thing: `do_control(); usleep(1000);`. On the bench the motor sang. In the field, a data logger running alongside it showed the loop at **930 Hz**, never 1000, and the frequency wandered with CPU load. Nothing was "slow". Every cycle was simply *1 ms plus however long the work took*. The fix was one system call: sleep **until** a deadline, not **for** a duration.

### Relative sleep drifts

You met this in Fundamentals as `rtos_delay` vs `rtos_delay_until`. Linux has exactly the same trap:

```c
for (;;) {
    work();                 /* takes W */
    nanosleep(&period);     /* sleeps P, starting *after* the work */
}                           /* one cycle = W + P + wakeup latency */
```

With P = 5 ms and W = 0.3 ms, each cycle is 5.3 ms. After 1,000 cycles you're **300 ms** behind schedule, and the error grows forever. It also changes whenever W changes, so your "5 ms" loop jitters with the load.

The fix is an **absolute grid**: deadline<sub>k</sub> = start + k·P. Each cycle you sleep until the next grid point. Work time and wakeup latency now delay *that* cycle, but they **don't accumulate**.

```c
struct timespec next;
clock_gettime(CLOCK_MONOTONIC, &next);
for (;;) {
    next = ts_add_ns(next, period_ns);                       /* the next grid point */
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    work();
}
```

That's `vTaskDelayUntil`, spelled POSIX. `ts_add_ns` is your job in the lab, and it's fiddly: `tv_nsec` must stay in `[0, 1e9)`, so 0.6 s + 0.5 s carries into `tv_sec`, and a negative offset has to *borrow*. Careful here: in C, `-7 % 3` is `-1`, not 2.

### Which clock?

- **`CLOCK_REALTIME`** is wall-clock time. NTP, GPS or an admin's `date -s` can **step** it, backwards or forwards. An absolute deadline on REALTIME can fire instantly, or an hour late, when the clock jumps.
- **`CLOCK_MONOTONIC`** counts steadily from boot and never steps. NTP may *slew* its rate slightly, but it never jumps. Use it for all periodic work and timeouts.
- (`CLOCK_BOOTTIME` is monotonic but includes suspend, for "wake me in 10 minutes even if we sleep".)

The rule: REALTIME is for **timestamps humans read**, and MONOTONIC is for **intervals and deadlines**.

### timerfd: the periodic timer as a file descriptor

`clock_nanosleep` blocks the whole thread. If your loop also waits on sensors and a shutdown eventfd (the last node), you want the *tick* to be an fd too:

```c
int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
struct itimerspec spec = {
    .it_value    = first_deadline,     /* absolute, with TFD_TIMER_ABSTIME */
    .it_interval = period,             /* then every period, on the kernel's grid */
};
timerfd_settime(tfd, TFD_TIMER_ABSTIME, &spec, NULL);

uint64_t expirations;
read(tfd, &expirations, sizeof expirations);   /* blocks until the next tick */
```

The interval timer keeps its own grid in the kernel, so it doesn't drift. And `read()` returns an 8-byte **expiration count**: how many periods elapsed since your last read.

- `1` means you're on time.
- `4` means you were late, the timer fired four times while you were busy, and you **missed three periods**.

That count is the honest answer to "did my real-time loop keep up?". A `nanosleep` loop can't tell you: it just quietly runs late.

### Worked example: reading the counter

Period 5 ms. Cycle 2's work hits a 23 ms stall (a page fault, a log flush to SD):

```
t=10  read → 1   cycle 1
t=15  read → 1   cycle 2  … stalls until t≈38
t=38  read → 4   (ticks at 20, 25, 30, 35 all happened)  → overruns += 3
t=40  read → 1   back on the grid
```

`expirations` is now `cycles + overruns`. Log it, count it and alarm on it. In a control system, a missed period is a fault, not a performance statistic.

### What to do about overruns

There's no universal answer, so pick a policy deliberately:

- **Skip** (what timerfd gives you naturally): run once, notice `count > 1`, and carry on from the next grid point. This is right for control loops, where stale cycles are useless.
- **Catch up**: run the work `count` times, which suits integrators or sample counters that must stay consistent. It's dangerous if work > period, because you'll never catch up.
- **Degrade or fault**: after N consecutive overruns, switch to a safe state, the same as a watchdog in Fundamentals.

### Gotchas

- **Never mix clocks.** Build the deadline with `clock_gettime(CLOCK_MONOTONIC)` and sleep on `CLOCK_MONOTONIC`.
- **Normalise every time.** `tv_nsec = 1000000000` is invalid, and `timerfd_settime` and `clock_nanosleep` reject it with `EINVAL`.
- **Re-arming a one-shot timer with a relative time each cycle** brings the drift back. Use `it_interval`, or absolute deadlines.
- **`read()` on a timerfd must ask for exactly 8 bytes**, or it fails with `EINVAL`.
- **The first cycle is special.** Arm it at now + P, so the grid starts from a known point.
- **Close the timerfd.** A loop that runs per job and leaks one fd per run eventually runs out.

### In the wild

- **Robot controllers** (ROS 2 `ros2_control`, EtherCAT masters such as IgH/SOEM) run 1 kHz loops on `clock_nanosleep(TIMER_ABSTIME)` or timerfd, and publish overrun counters as diagnostics.
- **Audio servers** (PipeWire, JACK) are driven by timerfds and count "xruns", which are overruns by another name.
- **systemd timers and GLib's `g_timeout_add`** use timerfd under the hood, and `CLOCK_BOOTTIME` handles laptops that suspend.
- **Interview insight:** "Why does `usleep(1000)` in a loop not give you 1 kHz?" Because a relative sleep adds the work time and the wakeup latency to every cycle, so the error accumulates. Use an absolute deadline on CLOCK_MONOTONIC, and watch the timerfd expiration count to *detect* misses. Mentioning that REALTIME can step under NTP marks you as someone who has shipped this.
