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
make test-m1        # one milestone
make test           # everything
make CFLAGS="-g -O1 -fsanitize=address,undefined" test   # with sanitizers
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
