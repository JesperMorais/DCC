# Sump pump controller on real FreeRTOS

A small plant controller running on the **real FreeRTOS kernel**, compiled for Linux with its POSIX port. The milestones (in the app) say what each part must do.

## Build and run

You need `gcc`, `make` and `git`. The first `make build` clones the FreeRTOS kernel (tag `V11.3.1`) into `third_party/`. After that it builds offline.

```sh
make build
./build/plant                                   # the default 3000 ms run
./build/plant --run-ms 2000 --estop-at 1537     # press the e-stop at tick 1537
./build/plant --sensor-stall-at 1350            # the level sensor dies at tick 1350
```

The program prints one line per event, `<tick> <TAG> key=value ...`, for example `300 SENSOR level=706`. When it exits, the simulated board adds its own report on stderr (`hw: ...`).

## Test it

```sh
make test-m1    # one milestone
make test       # everything (takes about 10 seconds: each test runs the real program)
```

The tests never link against your code. They run `./build/plant` with scenario options, read what it printed and the board's report, and check the behaviour. Any structure that behaves right passes. If you're unsure about a requirement, the checks are in `tests/m1.c` … `tests/m4.c`.

All of `src/` is compiled with `-std=gnu17 -Wall -Wextra -Werror`. The kernel in `third_party/` is compiled without `-Werror`, since it isn't your code.

## Files

- `src/plant.h`: the data types and constants (given). Use them, and add your own headers.
- `src/hw.h`, `src/hw.c`: the simulated board (given, don't edit). Read `hw.h` like a datasheet.
- `src/FreeRTOSConfig.h`: the kernel configuration (given).
- `src/hooks.c`: the kernel's assert, malloc-failed and idle hooks (given).
- `src/main.c`: yours. The Makefile compiles every `.c` file in `src/`, so split it up however you like.

## Things that are different on the POSIX port

- **Every task is a pthread**, but the port lets only one of them run at a time, so the scheduling is still FreeRTOS's. A task's real stack is its pthread's stack. That's why the stack sizes you pass to `xTaskCreate` barely matter here, and why stack overflow checking is off.
- **The tick is a signal.** A helper thread sends `SIGALRM` to the running task's thread every millisecond. The handler is the tick ISR. "Interrupts disabled" means "signals blocked", which is what `taskENTER_CRITICAL()` does.
- **The e-stop interrupt is a signal too** (`SIGUSR2`, raised by the board). Your ISR runs inside its handler, on the thread of whichever task was running. Only `...FromISR` APIs are allowed there, just like on a microcontroller. The board aborts with a message if you call `hw_pump_set()` or `hw_read_level()` from it.
- **Only one task may use `printf`.** stdio takes a pthread lock. If the kernel switches away from a task that's holding it, the next task that prints can hang. Send log lines to one logger task instead.
- **Ticks are not perfectly regular.** Linux isn't a real-time OS, so the tick count can occasionally jump by one. The tests allow a tick or two of slack where timing matters.
