Your gateway reads three serial sensors (temperature, humidity, pressure) and must also shut down cleanly when systemd sends SIGTERM. Instead of one thread per sensor, you'll write **one thread with one `epoll` loop**. In the tests, the sensors are pipes.

```c
typedef bool (*ev_handler)(int fd, void *ctx);   /* return false to stop watching fd */

struct evloop *evloop_create(void);
int  evloop_add(struct evloop *loop, int fd, ev_handler fn, void *ctx);
int  evloop_request_stop(struct evloop *loop);
int  evloop_run(struct evloop *loop);
void evloop_destroy(struct evloop *loop);
```

**`evloop_create`** creates an epoll instance and an `eventfd` used as the shutdown signal. It returns `NULL` on failure.

**`evloop_add`** watches `fd` for input, with at most `EVLOOP_MAX` (16) fds.
- It returns 0, or `-ENOSPC` when the loop is full, or `-errno` from `epoll_ctl`. That last case covers `-EBADF` for a closed fd and `-EPERM` for a regular file, which epoll can't watch.

**`evloop_request_stop`** writes to the eventfd. It's safe to call from another thread, and calling it more than once is fine. It returns 0 or `-errno`.

**`evloop_run`** runs the loop. It returns the **number of handler calls**, or `-errno`.
- It **sleeps** in `epoll_wait` until something happens. No busy polling: one test checks that 60 ms of waiting costs almost no CPU.
- When a watched fd is readable, or hung up, or in error, it calls that fd's handler. The handler does the `read()`.
- It uses **level-triggered** mode, so a handler that reads only part of the data is called again.
- If a handler returns `false` (for example on EOF), it stops watching that fd. It never closes it, because the fd belongs to the caller.
- **Graceful shutdown:** once the stop eventfd fires, it keeps dispatching until **no** watched fd is ready, and only then returns. Data that's already in the pipes is never dropped.

**`evloop_destroy`** closes the loop's own fds (the epoll fd and the eventfd) and frees the loop.

The tests are deterministic: they usually write into the pipes and request the stop **before** calling `evloop_run`.

```
pipes hold "temp=41", "hum=55", "press=1013"; stop requested
evloop_run(loop)   → 3   (one handler call each, then it exits)

pipe holds 40 bytes, the handler reads 4 at a time; stop requested
evloop_run(loop)   → 10  (it drains everything before it exits)
```
