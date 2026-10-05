# How we'd structure it

```
src/
  config.c/.h   file → struct config, or "file:line: why"
  channel.c/.h  IIO directory → double
  stream.c/.h   bytes → lines → samples and this period's min/mean/max/count
  csvlog.c/.h   open/append/rotate sensord.csv, knows its row count
  control.c/.h  listening socket + a fixed table of client slots
  daemon.c/.h   setup, the epoll loop, shutdown: the only file that knows about fds in epoll
  util.c/.h     trim, write_all, read_small
  main.c        argv → --once or daemon_run, exit codes
```

Everything except `daemon.c` is plain C with no event loop in sight. `stream_feed(&s, buf, n)` doesn't care whether the bytes came from a FIFO, a UART or a unit test, and `config_load` serves startup, `--once` and SIGHUP alike.

## Decision 1: one loop, one thread, tagged fds

```c
enum { TAG_SIGNAL = 1, TAG_TIMER, TAG_FIFO, TAG_LISTEN, TAG_CLIENT = 100 };
struct epoll_event ev = { .events = EPOLLIN, .data.u64 = TAG_CLIENT + slot };
```

The dispatch is a plain `if` chain on the tag. No locks, because nothing runs concurrently, and nothing blocks except `epoll_wait`: every fd is non-blocking. Signals are blocked with `sigprocmask` *before* anything else is set up, so a SIGHUP during startup can't terminate the process.

## Decision 2: the FIFO always has a reader

When the last writer leaves, the read end reports EOF forever. Close-then-reopen leaves a gap in which a new writer gets `EPIPE`. We open the new read end first, register it, and only then remove and close the old one. A fresh read end doesn't report EOF until a writer has come and gone again.

The other common answer is opening the FIFO `O_RDWR`, so sensord counts as a writer and never sees EOF. It works on Linux, but POSIX leaves it undefined, and it hides "the sensor went away", which on a real UART you want to notice.

## Decision 3: shutdown is a protocol

```c
while (!d->stopping) { /* epoll_wait, dispatch */ }
read_fifo(d);              /* whatever is still in the pipe */
stream_finish(&d->stream); /* a half line is an error */
d->tick++;
write_row(d);              /* the unfinished period */
```

Then one cleanup block closes whatever is `>= 0` and unlinks the socket. Setup failures end up there too: one exit path to keep correct.

## Decision 4: reload is "validate, then commit"

`config_load` fills a local struct and copies it out only on success, so a broken file can't leave a half-applied config. SIGHUP loads into `next`, refuses to change `fifo`/`log_dir`/`control` (they'd need fds reopened), and re-arms the timer only if the period changed.

## Smaller things worth noticing

- **The grid:** `TFD_TIMER_ABSTIME` with an absolute first expiry and `it_interval`. The `u64` you read says how many periods passed, so `tick += expirations` makes missed periods visible in the log.
- **Rows go out with `write(2)`**, not into a stdio buffer, so a SIGKILL loses nothing already logged. On a real board you'd add `fdatasync` every N rows, trading flash wear against durability.
- **Rotation happens lazily**, right before the row that wouldn't fit, so "exactly N rows per rotated file" falls out by itself.
- **Client slots are a fixed array.** A 17th client, a line longer than the buffer or a client that won't read its replies gets dropped. Nothing grows without bound in a program that runs for a year.
- **`lstat` before `unlink`:** deleting whatever sits at a configured path is how daemons eat config files.

Where next? Several UARTs (an array of streams, one tag each), `sd_notify` with `WatchdogSec=` from the tick, or gzipping rotated files in a worker thread fed by a bounded queue, so sampling never waits for compression.
