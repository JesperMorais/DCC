The first version of a factory gateway had one thread per serial sensor: twelve sensors, twelve threads, twelve blocking `read()` calls. Then came the bug reports. Shutdown hung, because one thread sat in `read()` on a sensor that never spoke. A config reload raced with three threads at once. Then a customer plugged in 40 sensors. The rewrite used **one thread and one `epoll` loop**, and the bugs simply had nowhere left to live.

### The problem: waiting on many things at once

A plain `read()` on a pipe, socket or serial port **blocks**. That's great when you have one input and terrible when you have twelve, because while you're blocked on sensor 3, sensor 7's data piles up.

You know this from Fundamentals. A task blocked on *one* queue can't also wait on another. The superloop solved it by checking every flag in turn, and wasted CPU spinning when nothing was happening. What you really want is: **"sleep until any of these is ready, then tell me which."**

There are three ways to get it:

- **Non-blocking I/O** (`O_NONBLOCK`): `read()` returns `-1`/`EAGAIN` instead of waiting. On its own that's just a superloop that busy-polls, burning a core.
- **`select()` / `poll()`**: pass in a list of fds and sleep until one is ready. The catch is that you pass the *whole list* on every call and the kernel scans it every time, which is **O(n) per wakeup**. That's fine for 10 fds and painful for 10,000.
- **`epoll`**: register each fd **once** with `epoll_ctl`. `epoll_wait` sleeps and returns *only the ready ones*. The cost scales with activity, not with how many fds you watch.

```c
int ep = epoll_create1(EPOLL_CLOEXEC);
struct epoll_event ev = { .events = EPOLLIN, .data.ptr = my_sensor };
epoll_ctl(ep, EPOLL_CTL_ADD, fd, &ev);              /* once */

struct epoll_event ready[16];
int n = epoll_wait(ep, ready, 16, -1);              /* -1 = sleep until something happens */
for (int i = 0; i < n; i++) handle(ready[i].data.ptr);
```

`data` is yours. Put a pointer to your per-fd state in it and dispatch through that. This is the superloop done right: a loop, but one that **sleeps in the kernel** until there's work.

### Level- vs edge-triggered

- **Level-triggered** (the default) reports an fd as ready **as long as** it has unread data. Read only 4 of 40 bytes and the next `epoll_wait` reports it again. This is forgiving, and it's the right default.
- **Edge-triggered** (`EPOLLET`) reports it **once, when it becomes ready**. If you don't drain it, you never hear about the rest of the data until *new* data arrives. With `EPOLLET` you **must** use `O_NONBLOCK` and read in a loop until `EAGAIN`.

It's the difference between a level-sensitive and an edge-sensitive interrupt line from Fundamentals, with the same bug: miss the edge and you wait forever.

### eventfd: a counting semaphore with a file descriptor

How do you wake an epoll loop from *another thread*, or from a signal handler, to say "shut down" or "config changed"? You give it something to wait on. `eventfd()` creates a kernel 64-bit counter wrapped in an fd:

- `write(efd, &one, 8)` adds to the counter and makes the fd readable.
- `read(efd, &val, 8)` returns the counter and resets it to 0 (non-semaphore mode).

That's your Fundamentals **counting semaphore**: give from anywhere (`write` is async-signal-safe, so even a SIGTERM handler can call it), and take inside the loop. And because it's an fd, it sits in the same `epoll_wait` as your sensors. A related trick is `signalfd`, which turns signals themselves into readable fds, so there's no handler at all.

### Worked example: a graceful shutdown

The pipes already hold data when SIGTERM arrives. What should happen?

```
epoll_wait → [sensor0 readable, stop eventfd readable]
  dispatch sensor0
  stop: read the eventfd (reset it), stopping = true
epoll_wait(timeout 0) → [sensor0 still readable]   ← level-triggered!
  dispatch sensor0
epoll_wait(timeout 0) → nothing ready
  return                                           ← drained, now exit
```

Once stopping, a timeout of **0** turns `epoll_wait` into "what's ready right now?" and never sleeps. You exit when the answer is "nothing", so no buffered data is lost. A loop that returns the moment it sees the stop event drops whatever the sensors had already sent.

### Gotchas

- **EOF looks like "ready".** When the writer closes a pipe, the read end reports `EPOLLHUP` (plus `EPOLLIN` while unread data remains), and `read()` returns 0, **forever**. If you don't `EPOLL_CTL_DEL` the fd, a level-triggered loop spins at 100 % CPU. Handle `EPOLLHUP`/`EPOLLERR` as well as `EPOLLIN`, and let the handler's `read()` discover the EOF.
- **Regular files can't be watched.** `epoll_ctl` returns `EPERM` for them, because they're always ready.
- **Registrations belong to the open file, not the fd number.** `close()` only removes the registration if no other fd (a `dup`, or a copy in a forked child) still refers to the same open file. Otherwise epoll keeps reporting events for an fd number you've closed, and that number may already belong to something else. `EPOLL_CTL_DEL` before you close.
- **Ownership.** The loop owns its epoll fd and its eventfd and must close them. The fds it watches belong to the caller.
- **`EINTR`.** `epoll_wait` can return `-1`/`EINTR` when a signal arrives. Treat that as "loop again", not as a fatal error.
- **Busy-polling in disguise.** `epoll_wait(..., 0)` in a loop where nothing is stopping is a superloop with extra steps. Sleep with `-1`, or with a real timeout.

### In the wild

- **nginx, Redis, Node.js (libuv), HAProxy**: one thread and one epoll loop serving tens of thousands of connections. That's how C10k got solved.
- **systemd, D-Bus, NetworkManager** run their main loops on `sd-event`/GLib, which wrap epoll with signalfd, timerfd and eventfd all in the same set.
- **Embedded gateways** (Modbus/serial bridges, CAN loggers): sensor fds, a socket to the cloud, a timerfd for heartbeats and an eventfd for shutdown, all in one thread, with no locks needed.
- **Interview insight:** "Level vs edge triggered?" Level re-reports while data remains. Edge reports transitions, so you must drain with non-blocking reads until `EAGAIN`, or you'll stall. Follow-up: "Why is epoll faster than poll?" Registration is persistent and the kernel keeps a ready list, so `epoll_wait` costs O(ready), not O(watched).
