The rig had passed every bench test. Then, on its first night on the factory floor, someone tripped over sensor 3's cable. The daemon's CPU usage went to 100%, the 10 ms sampling jitter grew to 400 ms, and by morning the log held eleven million identical lines. The read end of an unplugged serial line is *always readable*. It reports EOF forever, and the loop dutifully "handled" it a million times a second.

Nothing in this boss is new. It's every piece of this branch in one process, with the failure modes left in.

### The shape of a real daemon

Almost every embedded Linux data logger, PLC gateway and telemetry agent has the same skeleton:

```
             ┌────────── epoll_wait() ──────────┐
             │                                  │
   timerfd ──┤  every period: sample, average,  │
             │  write a line                    │
 sensor fds ─┤  bytes in → line buffer → ring   │
             │                                  │
   eventfd ──┤  shutdown: drain, flush, exit    │
             └──────────────────────────────────┘
```

There's **one thread**, and it never sleeps anywhere except in `epoll_wait`. Every source of work is a file descriptor. That's the superloop from Fundamentals with the busy-waiting removed: the kernel wakes you only when something has happened.

Map it back:

| Fundamentals | This daemon |
|---|---|
| A periodic task with `rtos_delay_until` | `timerfd` with `TFD_TIMER_ABSTIME`, and the expiration count tells you about overruns |
| A UART ISR filling a ring buffer | A sensor fd, plus a per-sensor line buffer |
| A queue between producer and consumer | The per-sensor ring of the last 8 samples |
| A shutdown semaphore | The `eventfd`, written by a SIGTERM handler or by another thread |

### Worked example: one tick in the life

Say the period is 10 ms and two sensors are connected. Here's one tick, step by step:

1. **t = 3 ms:** sensor A's fd is readable. You `read()` 64 bytes and get `"21\n22\n2"`. That makes two complete lines (21 and 22), which go into A's ring. The `"2"` waits in A's **line buffer**: it's half a number, not an error.
2. **t = 6 ms:** A again, this time `"3\n"`. The line buffer now holds `"23"`, which completes, so 23 goes into the ring.
3. **t = 10 ms:** the timerfd is readable. You read the count and get 1, which means you were on time. You compute the mean of each ring and append `T 1 22.0 -` to the log. Sensor B hasn't sent anything yet, so it shows as `-`.
4. **t = 14 ms:** the eventfd is readable because systemd sent SIGTERM. You **don't** exit yet. You switch `epoll_wait` to a timeout of 0 and keep handling whatever is already queued. When a wait comes back empty, you write `END …`, close what you opened, and return.

Note what never happens: no `sleep()`, no polling with a timeout "just in case", and no second thread.

### The four things that break in production

**1. Lines don't respect `read()` boundaries.** A pipe, a UART or a TCP socket gives you bytes, not messages. Whatever arrives after the last `\n` belongs to the next line, so it has to survive until the next read, in a buffer that belongs to **that sensor**. One shared buffer for all sensors interleaves their half-lines, and that's a bug you only see under load.

**2. EOF is a level, not an event.** Once the writer is gone, the read end stays readable forever, with `EPOLLIN|EPOLLHUP` and `read()` returning 0. If you don't `EPOLL_CTL_DEL` it, you get the 100% CPU story from the hook. The fix is one line, and forgetting it costs a night.

**3. Garbage is data too.** EMI on a long cable produces bytes like `"\x00\xff12"`. A sensor in fault mode prints `ERR`. Count these, skip them, and **bound** the line buffer. A sensor that never sends `\n` must not grow your memory, or overflow it.

**4. Shutdown is a protocol, not `exit(0)`.** Samples already sitting in the pipes belong in the log, which is why you drain first, then write the final summary, then clean up. Clean up means close what *you* opened: the epoll fd, the timerfd and the file. The sensor fds and the eventfd belong to the caller, and closing them is as wrong as leaking your own.

### Ownership, in one rule

> Whoever opens it closes it, on **every** path.

Write the function so there's one exit path. Initialise the fds to `-1`, run `setup()` and then `loop()`, and fall through to a single cleanup block that closes whatever is `>= 0`. A daemon that leaks one fd per reconnect runs out of fds (`EMFILE`) after about a thousand reconnects. At one reconnect a minute, that's a field failure about 17 hours after deployment, long after your demo went fine.

### Gotchas

- **Use a tag in `epoll_event.data`.** A small integer (the sensor index, or a special value for the timer) saves a lookup and keeps the dispatch a plain `if`/`else`.
- **Read the timerfd's count.** If you skip the read, the timer stays readable and you spin. If you ignore the value, you never learn that you missed periods.
- **`%.1f` of an integer mean** needs a cast to `double` first, because `sum / count` with two integers truncates 8.5 to 8.
- **Write averages from the tick, not from the sensor handler.** The log's timebase is the timer, not whenever bytes happen to arrive.

### In the wild

- **systemd services** look exactly like this. `Type=notify` plus `WatchdogSec=` means you also call `sd_notify("WATCHDOG=1")` from the tick handler. If the loop wedges, systemd restarts you. Stopping a service sends SIGTERM, and a `signalfd` (or a handler that writes to an eventfd) turns it into one more fd in the loop.
- **Industrial gateways** (Modbus/RTU to MQTT, CAN loggers, Victron's Venus OS on solar installs) are single-threaded event loops over serial fds and timers. Many are built on libevent, libuv or sd-event, which wrap this same epoll loop.
- **Interview question:** *"Design a logger for 4 sensors at 100 Hz on a Cortex-A7. Threads or epoll?"* A strong answer: one epoll loop, because the work per event is tiny and a single thread has no locks. Use absolute-time ticks, handle EOF and garbage, and shut down with a drain. Mention when you'd add a thread: when one event's handling is slow (compression, uploads). Hand that work to a worker over a bounded queue, the one you built in l-pthreads, so the sampling loop never blocks.
