# sensord

A sensor logging daemon for embedded Linux, built on plain Linux APIs: a FIFO for a streaming sensor, an IIO-style sysfs directory for a polled one, `epoll`, `timerfd`, `signalfd`, rotated CSV logs and a UNIX control socket. It runs on any Linux PC, no root and no hardware needed. The milestones (in the app) describe what it must do.

## Build and run

```sh
make build                  # builds ./sensord from every .c file in src/
./sensord --once sensord.conf
./sensord sensord.conf      # runs in the foreground until SIGTERM or Ctrl-C
```

A config to play with (paths are relative to where you start it):

```
fifo = sensor.fifo
sysfs = iio
log_dir = logs
period_ms = 1000
```

and a fake temperature channel:

```sh
mkdir -p iio
echo 23500 > iio/in_temp_raw
echo 0.001 > iio/in_temp_scale
```

Then, from another terminal: `echo 42 > sensor.fifo`, `tail -f logs/sensord.csv`, `kill -HUP <pid>`, and, once milestone 5 works, `socat - UNIX-CONNECT:sensord.sock` (or `nc -U sensord.sock`) and type `stats`.

## Test it

```sh
make test-m1                    # one milestone
make test-m2 T=reconnect        # only the m2 tests whose name contains "reconnect"
make test                       # everything
make clean && make CFLAGS="-g -O1 -fsanitize=address,undefined" test   # with sanitizers
```

The build always uses `-std=gnu17 -Wall -Wextra -Werror -D_GNU_SOURCE`, so a warning is an error.

The tests are black-box. Each one makes a fresh temp directory, writes a config and a fake sysfs channel, starts `./sensord` there, writes to its FIFO, sends it signals, talks to its socket and reads the CSV files it produces. They never include your code, so any structure passes. Output is TAP: `ok 3 - name` or `not ok 3 - name` with the reason on the `#` lines above it, and sensord's stderr is shown when a test fails. Read a test in `tests/m*.c` when you're unsure what's expected.

Timing checks are generous on purpose (your PC isn't a real-time system), so a test that fails is failing for a real reason. If one fails only sometimes, look for a race: what happens if a signal, a byte or a client arrives at an unlucky moment?

## On a real board

The design carries over almost unchanged; mostly the paths move:

- **The polled sensor** becomes a real IIO device, for example `sysfs = /sys/bus/iio/devices/iio:device0`. Same files, same `(raw + offset) * scale` formula; real temperature channels give milli-degrees Celsius. For fast sampling you'd switch to the IIO buffer interface (`/dev/iio:device0`, which *can* go in epoll, triggered by the hardware).
- **The streaming sensor** becomes a UART such as `/dev/ttyS1`: opened non-blocking the same way, plus a `termios` setup for the baud rate. A serial line has the same problems: half lines, garbage on a long cable, and a device that disappears (an unplugged USB-serial adapter reports `EPOLLHUP` and read errors like `EIO` instead of a FIFO's EOF).
- **A GPIO** (a "data ready" pin, an alarm LED) goes through the GPIO character device with libgpiod, not `/sys/class/gpio`. A line request gives you an fd that becomes readable on an edge, so it slots into the same epoll loop.
- **Running it**: a systemd unit with `Type=simple`, `ExecReload=/bin/kill -HUP $MAINPID`, and `Restart=on-failure`. `systemctl stop` sends SIGTERM, `systemctl reload` SIGHUP. Logs go to an eMMC or SD card, which is why rotation matters.

## Files

- `src/main.c`: a stub so `make build` works. Replace it and add as many files as you like.
- `tests/`: `m1.c` … `m5.c` (one per milestone), `harness.c` (starts sensord, reads its files), `check.h` (the tiny TAP helper) and `run.c` (the runner).

The structure is yours to design. If you want a starting point, think about which parts of the program could be tested without a running daemon at all.

## If you want a start

Entirely optional: one layout that works. Each file has one job, and the ones marked *no I/O* can be tried out from a tiny test program of your own.

```
src/main.c        argv → --once or the daemon; turns results into exit codes
src/config.c/.h   load and validate the config into a struct (no daemon needed)
src/channel.c/.h  read the IIO-style temperature directory
src/stream.c/.h   bytes in → lines → samples → the period's count/min/mean/max (no I/O)
src/csvlog.c/.h   open the log, append a row, rotate (m4)
src/daemon.c/.h   the epoll loop: setup, one handler per fd, shutdown
src/control.c/.h  the control socket's clients (m5)
```

And a `main.c` that only dispatches, so the real work lives in functions:

```c
#include <stdio.h>
#include <string.h>

/* Both return the exit code: 0 when it worked, 1 when it didn't. */
static int run_once(const char *config_path) {
    /* m1: load the config, read the channel, print temp=…, return 0 */
    (void)config_path;
    return 1;
}

static int run_daemon(const char *config_path) {
    /* m2: set up the fds, loop until told to stop, clean up */
    (void)config_path;
    return 1;
}

int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[1], "--once") == 0) return run_once(argv[2]);
    if (argc == 2 && argv[1][0] != '-') return run_daemon(argv[1]);
    fprintf(stderr, "usage: sensord [--once] <config>\n");
    return 2;
}
```

Before milestone 2, draw the file descriptors on paper: what each one is, who opens it, who closes it, and what wakes it up.

## When you're stuck

1. **Read the first failing test only.** Later failures are often the same bug. The lines above `not ok` say what went wrong:

   ```
   #   tests/m2.c:174: failed: enxio == 0
   #   534 of the writers found nobody reading sensor.fifo (open failed with ENXIO)
   not ok 9 - writers_never_find_the_fifo_unread
   ```

   The first line is the file and line of the check and the expression that was false. Open `tests/m2.c` at that line and read the test from the top: it's a short story of what the test did to sensord (wrote a config, started it, wrote to the FIFO, sent a signal) and what it expected. The second line is the explanation, usually "got this, expected that". If sensord printed anything to stderr, it's shown there too, under `sensord's stderr:`.
2. **Run just that milestone** (`make test-m2`), or one test by name (`make test-m2 T=writers_never`).
3. **Look at the value.** `fprintf(stderr, ...)` anything you're unsure of: stderr is shown under a failing test, and stdout only matters for `--once`. Or replay the test by hand in two terminals (see Build and run). For an fd problem, watch the syscalls:

   ```sh
   strace -f -e trace=desc ./sensord sensord.conf
   ```

   Every `read`, `write`, `epoll_wait` and `openat` appears with its result, for example `read(5, "", 4096) = 0` (EOF) or `= -1 ENXIO`. The same `read(...) = 0` scrolling by forever is an EOF spin. `ls -l /proc/$(pgrep -n sensord)/fd` lists every open fd (`anon_inode:[eventpoll]`, `[signalfd]`, `[timerfd]`, your FIFO and log): compare it before and after ten clients to find a leak. For a crash, `gdb --args ./sensord sensord.conf`, then `run`, and `bt` when it stops. strace and gdb don't mix with the sanitizers, so `make clean && make build` first.
4. **Make the step smaller.** In milestone 2, first make rows appear with count 0. Then count bytes from the FIFO. Then lines, then numbers, then EOF. Run the tests after each step.
5. **Take a hint.** Hints are a tool, not a failure. They point at the lesson to reread.
6. **Commit when green.** `git init` once, then `git add -A && git commit -m "m2 green"` after each milestone, so you can always get back to a working version.
7. **Walk away for ten minutes.** Seriously. Most bugs are found on the way back.
