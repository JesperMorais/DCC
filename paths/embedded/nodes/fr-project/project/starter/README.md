# Sump pump controller on real FreeRTOS

A small plant controller running on the **real FreeRTOS kernel**, compiled for Linux with its POSIX port. The milestones (in the app) say what each part must do.

There's no clock on this project. Taking an evening per milestone, or longer, is completely normal. Every time you're stuck and work your way out, you're learning the part that makes you able to do this on a real board.

## Build and run

You need `gcc`, `make`, `git` and (for debugging) `gdb`. The first `make build` fetches the FreeRTOS kernel (tag `V11.3.1`) into `third_party/`. After that it builds offline.

```sh
make build
./build/plant                                   # the default 3000 ms run
./build/plant --run-ms 2000 --estop-at 1537     # press the e-stop at tick 1537
./build/plant --sensor-stall-at 1350            # the level sensor dies at tick 1350
```

The program prints one line per event, `<tick> <TAG> key=value ...`, for example `300 SENSOR level=706`. When it exits, the simulated board adds its own report on stderr (`hw: ...`).

Then, once:

```sh
git init && git add -A && git commit -m "starter"
```

`.gitignore` already leaves out `build/` and the fetched kernel.

## Test it

```sh
make test-m1              # one milestone
make test-m1 T=drift      # only the m1 checks whose name contains "drift"
make test                 # everything (about 10 seconds: each test runs the real program)
./build/run-tests m3 ESTOP    # the same, without make
```

The tests never link against your code. They run `./build/plant` with scenario options, read what it printed and the board's report, and check the behaviour. Any structure that behaves right passes. If you're unsure about a requirement, the checks are in `tests/m1.c` … `tests/m4.c`.

All of `src/` is compiled with `-std=gnu17 -Wall -Wextra -Werror`. The kernel in `third_party/` is compiled without `-Werror`, since it isn't your code.

## Files

- `src/plant.h`: the data types and constants (given). Use them, and add your own headers.
- `src/hw.h`, `src/hw.c`: the simulated board (given, don't edit). Read `hw.h` like a datasheet.
- `src/FreeRTOSConfig.h`: the kernel configuration (given).
- `src/hooks.c`: the kernel's assert, malloc-failed and idle hooks (given).
- `src/main.c`: yours. The Makefile compiles every `.c` file in `src/`, so split it up however you like.
- `gdbinit`: gdb settings for this port (see below).

## Where to start

Don't write all four tasks and then build. Get one thing running, check it, commit, add the next.

1. **Make `main()` start the scheduler** with a single task that sends one line and ends the run. Build and run it. A program that boots the real kernel and exits cleanly is already a milestone's worth of learning.
2. **Split the code by job, not by milestone.** For example `log.c` (the logger task and the log helpers), `plant.c` (sensor, control, and later the safety task and ISR), `watchdog.c` (milestone 4), and `main.c` (create everything, start the scheduler). The Makefile picks up new `.c` files by itself.
3. **Share through one header.** A handle that several files use is *defined* in exactly one `.c` file and *declared* `extern` in a header that the others include:

   ```c
   /* app.h */
   #include "FreeRTOS.h"
   #include "queue.h"
   #include "plant.h"

   extern QueueHandle_t g_readings;     /* defined once, in main.c: QueueHandle_t g_readings; */

   void log_task(void *arg);
   void log_at(TickType_t tick, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
   ```

   Without `extern` in the header, every file that includes it defines its own copy, and the linker complains about multiple definitions (or worse, you get separate variables).
4. **Write a log helper first.** Every milestone logs, so a printf-style helper that fills a `log_line_t` with `vsnprintf` and sends it to the logger's queue pays for itself immediately. The `format` attribute above makes gcc check your format strings the way it checks `printf`'s.

## Things that are different on the POSIX port

- **Every task is a pthread**, but the port lets only one of them run at a time, so the scheduling is still FreeRTOS's. A task's real stack is its pthread's stack. That's why the stack sizes you pass to `xTaskCreate` barely matter here, and why stack overflow checking is off.
- **The tick is a signal.** A helper thread sends `SIGALRM` to the running task's thread every millisecond. The handler is the tick ISR. "Interrupts disabled" means "signals blocked", which is what `taskENTER_CRITICAL()` does.
- **The e-stop interrupt is a signal too** (`SIGUSR2`, raised by the board). Your ISR runs inside its handler, on the thread of whichever task was running. Only `...FromISR` APIs are allowed there, just like on a microcontroller. The board enforces it: calling `xTaskNotify()`, `xQueueSend()`, `xSemaphoreGive()`, `xTaskGetTickCount()`, `taskENTER_CRITICAL()`, `hw_pump_set()` or another task-only function from the ISR aborts with a message that names the FromISR call to use. (That's also why you'll see `__wrap_...` frames from `hw.c` in backtraces: the Makefile routes those kernel calls through `hw.c`'s check.)
- **Only one task may use `printf`.** stdio takes a pthread lock. If the kernel switches away from a task that's holding it, the next task that prints can hang. Send log lines to one logger task instead.
- **`vTaskEndScheduler()` works here.** On a microcontroller the scheduler never returns. On the POSIX port, `vTaskEndScheduler()` stops the tick and makes `vTaskStartScheduler()` return in `main()`, which is how the program gets to exit with status 0 and let the board print its report. Flush stdout first.
- **Ticks are not perfectly regular.** Linux isn't a real-time OS, so the tick count can occasionally jump by one. The tests allow a tick or two of slack where timing matters.

## When you're stuck

1. **Read the first failing test only.** Each check prints `ok N - what it checks` or `not ok N - what it checks`, followed by a `#` line with what it actually saw: your END line, the board's `hw:` report, or how the program ended (`exited with 1`, `killed`, stderr). Compare that line with the check's name. Later failures are often the same problem seen again.
2. **Run just that milestone** (`make test-m2`), or one check by name (`make test-m2 T=mutex`). Or run the scenario yourself and read the output: the `#` line shows the exact arguments, e.g. `./build/plant --run-ms 2000 --estop-at 1537`.
3. **Look at the value.** Add a log line with your own tag (`DEBUG pump_on=1 level=712`): the tests ignore tags they don't check. Don't `printf` from tasks, but `fprintf(stderr, ...)` in `main()` before the scheduler starts is fine. For crashes, hangs and "which task is where", use gdb:

   ```sh
   gdb -x gdbinit --args ./build/plant --run-ms 2000 --estop-at 1537
   (gdb) break estop_isr          # or a file:line, e.g. break plant.c:42
   (gdb) run
   (gdb) info threads             # every task is a thread, named after the task
   (gdb) bt                       # how we got here
   (gdb) print some_variable
   (gdb) continue
   ```

   A `configASSERT failed at ...queue.c:1520` or an `hw: ... called from an ISR` message ends in `abort()`: run the same scenario under gdb and type `bt` when it stops on `SIGABRT`. The first frame in `src/` is your line. If the program hangs instead, press Ctrl-C in gdb, then `thread apply all bt` shows where every task is waiting.
4. **Make the step smaller.** One task, one queue, one line of output at a time. Build and run after each.
5. **Take a hint.** Hints are a tool, not a failure. They point at the lesson to reread.
6. **Commit when green.** `git init` once, then `git add -A && git commit -m "m2 green"` after each milestone, so you can always get back to a working version.
7. **Walk away for ten minutes.** Seriously. Most bugs are found on the way back.
