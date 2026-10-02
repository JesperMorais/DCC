### The clock that drifted a third of a second

On 25 February 1991, a Patriot battery in Dhahran failed to intercept an incoming Scud, and 28 soldiers died. The system had been running for about 100 hours. It counted time in tenths of a second, and 0.1 can't be represented exactly in binary, so a tiny error crept in with every count. Nothing ever corrected it. After 100 hours the clock was off by 0.34 s, and at Mach 5 that put the tracking gate more than half a kilometre away from the missile.

Small errors that **add up and never get corrected** are called *drift*. Your periodic tasks can drift too, and you won't see it in a five-second test run.

### The tick: the RTOS heartbeat

An RTOS keeps time with a hardware timer that interrupts at a fixed rate, typically 1 kHz. Each interrupt is a **tick**. On every tick the kernel:

1. increments the tick counter (`rtos_now()`),
2. wakes any task whose delay has expired,
3. checks whether a higher-priority task is now ready and, if so, preempts.

All the kernel's timing is in whole ticks. In our simulator 1 tick = 1 ms, and nothing happens *between* ticks except code that takes "zero time".

### Two ways to wait

`rtos_delay(10)` means "wake me 10 ticks **from now**". It is relative. `rtos_delay_until(&next, 10)` means "wake me at tick `next + 10`", and it advances `next` by exactly 10. It is absolute.

They look alike, but they behave very differently in a loop that does work:

```c
for (;;) {                    // relative: WRONG for periodic work
    sample(); rtos_busy(2);   // 2 ticks of work
    rtos_delay(10);           // 10 ticks after the work ENDS
}
```

```
tick   0    5    10   15   20   25   30   35   40
       |    |    |    |    |    |    |    |    |
delay  ##.........##.........##.........##......      period = 12, not 10
until  ##........##........##........##........##     period = 10, exactly
```

With `rtos_delay` the real period is *work + 10*, so every sample slides 2 ticks later than the one before. After an hour, you've lost 12 minutes' worth of samples. Worse, the error depends on how long the work took, so the spacing isn't even constant: a filter that sometimes takes 6 ticks turns your "100 Hz" into something between 62.5 and 83 Hz.

With `rtos_delay_until` the wake-up times are computed from a **grid** (0, 10, 20, ...), not from "now". Work that takes longer just means a shorter sleep:

```c
rtos_tick_t next = rtos_now();    // the grid starts here
for (;;) {
    sample(); rtos_busy(work);    // 2, 6, whatever
    rtos_delay_until(&next, 10);  // sleep until next += 10
}
```

That is the whole fix, and it's the single most important idiom in periodic firmware.

### Jitter vs drift

These are two different illnesses:

- **Jitter** is how much a single release deviates from its ideal tick. Suppose a higher-priority radio task is running at tick 30. The sampler then can't start until 34, so it has 4 ticks of jitter. Jitter is bounded, and the next release is on time again.
- **Drift** is an error that *accumulates*. With relative delays, every disturbance permanently shifts all future releases.

```
ideal   |    |    |    |    |
until   |    |      |  |    |        jitter: one release late, the grid holds
delay   |    |      |    |    |      drift: one late, every later one shifts
```

Control loops tolerate a little jitter. Drift ruins every downstream assumption: FFT bins, PID integrators, audio sample rates, and sync with the other board on the CAN bus.

Try it below. The `radio` is more urgent than the `sampler`. Make its `wcet` bigger and watch the sampler's start times wobble, while every job still finishes inside its own 10-tick window. Then push the radio's wcet past 8: the sampler starts missing deadlines, which is the next node's topic.

```playground
{
  "title": "Jitter from a higher-priority task",
  "ticks": 60,
  "editable": true,
  "tasks": [
    { "name": "radio", "priority": 3, "period": 15, "wcet": 3, "offset": 0 },
    { "name": "sampler", "priority": 2, "period": 10, "wcet": 2 }
  ]
}
```

### Overruns: when the work takes longer than the period

Suppose one iteration takes 13 ticks with a 10-tick period. The release at 30 has already passed when the work finishes at 33. `rtos_delay_until` sees that `next + 10` is in the past, advances `next` anyway and **returns immediately** (the timeline shows an `overrun` event). The late iteration runs at 33, then the task sleeps until 40, back on the original grid.

That's the FreeRTOS `vTaskDelayUntil` behaviour too. It is usually what you want for sampling, because the grid survives. Two other policies exist, and you'll meet both in real code bases:

- **Skip:** if you're late, drop the missed release(s) and wait for the next future grid point. That's best when a stale result is worthless, like a video frame.
- **Re-anchor:** `next = now` after an overrun. The grid moves, which is drift by another name. Only do this on purpose, for example after a long pause.

Whatever you choose, **count overruns**. An overrun counter in your telemetry is the cheapest early-warning system you'll ever build.

### Gotchas

- **Initialise `next` once, before the loop.** If you re-read `rtos_now()` every iteration, you've rebuilt `rtos_delay` with extra steps.
- **"Compensating" doesn't work.** `rtos_delay(10 - work)` breaks the moment you're preempted, because the time you were *not running* isn't in `work`. On an overrun it underflows into a delay of four billion ticks.
- **Delays are tick-quantised.** `rtos_delay(1)` called at tick 41.9 wakes at 42, only 0.1 ms later. A delay of N ticks really means somewhere between N-1 and N. If you need "at least 1 ms", ask for 2 ticks.
- **Tick rate is a trade-off.** At 1 kHz your timing resolution is 1 ms, and the CPU wakes 1000 times a second. Battery devices use *tickless idle*, which reprograms the timer to skip ticks while nothing is due. Then a periodic task is the reason the chip wakes up.
- **The tick counter wraps.** A 32-bit counter at 1 kHz wraps after 49.7 days. Compare times as `(int32_t)(a - b) > 0`, never `a > b`. Kernels handle it inside `delay_until`, but your own timeout code must handle it too.

### In the wild

- **FreeRTOS:** `vTaskDelayUntil(&xLastWakeTime, xPeriod)`, or `xTaskDelayUntil` which also tells you whether you overran. **Zephyr:** `k_sleep(K_TIMEOUT_ABS_MS(...))` or a periodic `k_timer`. **Linux:** `clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, ...)` or `timerfd`.
- **Flight controllers** (Betaflight, PX4) run the gyro loop at 4–8 kHz. They're triggered by the gyro's own data-ready interrupt, because even a perfect software timer drifts relative to the sensor's crystal.
- **Audio and motor control** measure jitter in microseconds, and they log overruns as a first-class health metric.
- **The Patriot fix** was a software update that corrected the clock arithmetic. It arrived in Dhahran one day after the attack.
