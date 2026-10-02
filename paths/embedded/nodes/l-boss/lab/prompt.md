Time to ship. A test rig on the factory floor has up to four sensors on serial lines, and each one streams ASCII readings, one per line. Your daemon samples them on a fixed period, logs moving averages, and survives everything the floor throws at it: garbage bytes, half-sent lines, unplugged cables, and a `systemctl stop` at any moment. It's one thread with one `epoll` loop, and no busy waiting.

```c
struct daq_config {
    const int *sensor_fds;  size_t n_sensors;   /* 1..4, read ends of pipes in the tests */
    int shutdown_fd;                            /* an eventfd: readable = shut down */
    int64_t period_ns;                          /* sampling period */
    const char *out_path;                       /* the log file to create */
};
struct daq_stats { unsigned long ticks, samples, errors, overruns; };

int daq_run(const struct daq_config *cfg, struct daq_stats *st);   /* 0 or -errno */
```

**Sensors.** Each sensor sends lines like `"123\n"` or `"-7\n"`.
- One line can arrive **split across several reads** (`"12"`, then later `"3\n"` is the value 123).
- Every valid integer line is one **sample**. It goes into that sensor's ring buffer, which keeps the **last 8** samples (`DAQ_WINDOW`).
- An empty line, a non-number, or a line of 32 or more characters (`DAQ_LINE_MAX`) counts as one **error** and is skipped.
- EOF or a read error means the sensor is unplugged. Count an unfinished line as one error, then **stop watching** that fd. Don't close it, because it isn't yours.

**Ticks.** A `timerfd` on `CLOCK_MONOTONIC` fires every `period_ns` on an absolute grid.
- On every tick, read the expiration count. Increment `ticks`, and add `count - 1` to `overruns`.
- Then append one line to the output file:
  ```
  T <ticks> <avg0> <avg1> ...
  ```
  Each average is the mean of that sensor's ring buffer, printed with `%.1f`, or `-` if the sensor has no samples yet. For example: `T 1 20.0 200.0`, or `T 1 8.5 -`.

**Shutdown.** When the eventfd becomes readable:
- First **drain** the sensors: keep handling events until nothing is ready, so no byte already in a pipe is lost.
- Then write the final line, `END ticks=<ticks> samples=<samples> errors=<errors>`, and return 0.
- If shutdown was requested before you even started, exit promptly. Don't wait for a tick.

**Errors and cleanup.**
- Return `-EINVAL` if `period_ns <= 0` or `n_sensors` isn't between 1 and 4. If creating the output file fails, return its `-errno` (for example `-ENOENT`). If registering an fd fails, return that `-errno` (`-EBADF` for a closed fd).
- On **every** path, close everything you opened (the epoll fd, the timerfd and the file), and nothing you didn't. The tests count open fds across 25 runs, and the leak check is on.

```
sensor A: "10\n20\n30\n"   sensor B: "100\n300\n"   10 ms period, stopped after ~75 ms
→ T 1 20.0 200.0
  T 2 20.0 200.0
  ...
  T 7 20.0 200.0
  END ticks=7 samples=5 errors=0
```

The tests load the pipes before `daq_run` starts and stop it from a helper thread. Tick counts are checked with a wide tolerance (3 to 12 ticks in 75 ms), but the file must contain exactly one `T` line per tick.
