### "It compiled in the simulator"

Every FreeRTOS lab so far ran in a simulator that speaks FreeRTOS's API. The next project uses the real kernel: the same `tasks.c` that ships in millions of devices, built with `make` on your Linux machine. Nothing new to learn about tasks or queues. What's new is the **build**, the **config file**, **real error messages** and **a real debugger**. This workshop walks through all four on the project's starter, so the first evening of the project goes to the pump and not to the Makefile. Copy the starter, keep a terminal open next to this page and try each step as you read. There's no hurry: one command, look at what it printed, then the next.

### What a FreeRTOS project is made of

Any FreeRTOS firmware, on a Cortex-M or on Linux, is four ingredients:

```
the kernel       tasks.c queue.c list.c timers.c        same on every chip
one port         portable/<compiler>/<cpu>/port.c       context switch, tick, critical sections
one heap         portable/MemMang/heap_N.c              how pvPortMalloc works (pick 1 of 5)
your config      FreeRTOSConfig.h                       what the kernel includes and how it behaves
```

The project's Makefile says exactly that:

```make
KERNEL_SRCS := $(KERNEL)/tasks.c $(KERNEL)/queue.c $(KERNEL)/list.c $(KERNEL)/timers.c \
               $(KERNEL)/portable/MemMang/heap_4.c $(PORT)/port.c $(PORT)/utils/wait_for_event.c
INCS    := -Isrc -I$(KERNEL)/include -I$(PORT) -I$(PORT)/utils
```

`PORT` is `portable/ThirdParty/GCC/Posix`. On an STM32 it would be `portable/GCC/ARM_CM4F`, and that's the only line that changes. `event_groups.c` and `stream_buffer.c` aren't listed because the project doesn't use them: leave a file out and its functions simply don't exist.

The include order matters too. `-Isrc` comes first because the kernel's `FreeRTOS.h` does `#include "FreeRTOSConfig.h"`, and that file is **yours**. `-I$(PORT)` provides `portmacro.h`, where `portYIELD_FROM_ISR` and `taskENTER_CRITICAL` turn into real code for this CPU.

Try it: `make build`. The first time, you'll see `fetching FreeRTOS-Kernel V11.3.1 into third_party/FreeRTOS-Kernel`, then one `gcc` line per file. Kernel files are compiled without `-Werror`, yours with it.

### FreeRTOSConfig.h, line by line

Open `src/FreeRTOSConfig.h`. The lines that shape your program:

- `configUSE_PREEMPTION 1`: a higher-priority task that becomes ready runs **now**, not when the current one yields. At 0 you'd have a cooperative scheduler.
- `configTICK_RATE_HZ 1000`: one tick is 1 ms, so `pdMS_TO_TICKS(100)` is 100. At 100 Hz, `pdMS_TO_TICKS(5)` would round down to 0.
- `configMAX_PRIORITIES 8`: priorities 0 to 7, with idle at 0. Asking for 8 trips an assert.
- `configTIMER_TASK_PRIORITY 6`: the `Tmr Svc` daemon. Anything at 7 outranks timer callbacks.
- `configTOTAL_HEAP_SIZE`: the array heap_4 carves every `xTaskCreate` and `xQueueCreate` out of. Run out and the malloc-failed hook fires.
- `INCLUDE_vTaskDelay`, `INCLUDE_xTaskDelayUntil`, …: switches that compile an API function in or out.
- `configASSERT(x)`: the kernel checks its arguments with this. Here it calls `vAssertCalled()` in `hooks.c`, which prints the file and line and aborts.

### Reading a build error

Set `INCLUDE_xTaskDelayUntil` to `0` in a finished project and build:

```
/usr/bin/x86_64-linux-gnu-ld.bfd: build/app/plant.o: in function `sensor_task':
/home/you/code/freertos-posix/src/plant.c:14:(.text+0x58): undefined reference to `xTaskDelayUntil'
collect2: error: ld returned 1 exit status
make: *** [Makefile:73: build/plant] Error 1
```

Read it from the top. `ld` means **the linker**: every file compiled fine, because `task.h` still declares the function, but `tasks.c` no longer defines it. The fix is a config switch, not your code. Compare a compile error, from forgetting `#include "timers.h"` in one file:

```
src/watchdog.c:15:25: error: unknown type name ‘TimerHandle_t’; did you mean ‘xTimerHandle’?
src/watchdog.c:34:23: error: implicit declaration of function ‘xTimerCreate’; did you mean ‘xTaskCreate’? [-Wimplicit-function-declaration]
src/watchdog.c:35:20: error: comparison between pointer and integer [-Werror]
```

(Shortened: gcc also prints each source line with a `^` under the spot.) Four more errors follow, and all seven have one cause, which the **first** one points at: `TimerHandle_t` is unknown, so a header is missing. Fix the top error, rebuild, and read again. `[-Werror]` means a warning was promoted to an error, which is why the project insists on it: `-Wimplicit-function-declaration` is a bug waiting to happen.

### What's different on the POSIX port

On a microcontroller, the port saves registers and swaps stacks. On Linux it can't, so it fakes the hardware:

- **Each task is a pthread**, but only one is ever allowed to run. The scheduling decisions are still FreeRTOS's.
- **The tick is `SIGALRM`**, sent to the running task's thread every millisecond. "Interrupts off" means "signals blocked".
- **Interrupts are signals.** The project's e-stop is `SIGUSR2`, and your ISR runs inside its handler on whichever task's thread was running.
- **Only one task may `printf`.** glibc's stdio takes a pthread lock. If the kernel switches away from a task holding it, the next task that prints waits forever on a lock FreeRTOS doesn't know about.
- **`vTaskEndScheduler()` makes `vTaskStartScheduler()` return.** That's POSIX-only. On a Cortex-M, `vTaskStartScheduler()` never returns. Here it's how the program exits cleanly.

### gdb on a running kernel

The project ships a `gdbinit` with `handle SIGALRM nostop noprint pass` and the same for `SIGUSR1` and `SIGUSR2`. Without it, gdb stops on every e-stop signal. With it:

```
$ gdb -x gdbinit --args ./build/plant --run-ms 2000 --estop-at 1537
(gdb) break estop_isr
(gdb) run
Thread 7 "IDLE" hit Breakpoint 1, estop_isr () at src/plant.c:54
(gdb) bt
#0  estop_isr () at src/plant.c:54
#1  0x00005555555568eb in estop_signal_handler (sig=<optimized out>) at src/hw.c:99
#2  <signal handler called>
...
#8  0x00005555555568b9 in vApplicationIdleHook () at src/hooks.c:27
(gdb) info threads
  2    Thread 0x7ffff7bff6c0 (LWP 2933538) "logger"   __syscall_cancel_arch () ...
  3    Thread 0x7ffff73fe6c0 (LWP 2933539) "sensor"   __syscall_cancel_arch () ...
* 7    Thread 0x7ffff53fa6c0 (LWP 2933543) "IDLE"     estop_isr () at src/plant.c:54
  8    Thread 0x7ffff4bf96c0 (LWP 2933544) "Tmr Svc"  __syscall_cancel_arch () ...
```

That's an interrupt, live: the ISR is running on the idle task's thread, on top of `<signal handler called>`. Each thread carries its **task name**, and `thread 3` then `bt` shows where the sensor is waiting.

A failed `configASSERT` looks like this. Forget to create the log queue, and the logger receives from a NULL handle:

```
$ ./build/plant --run-ms 500
configASSERT failed at third_party/FreeRTOS-Kernel/queue.c:1520
Aborted (core dumped)
```

The line points into the kernel, and the kernel isn't wrong. Run it under gdb, type `bt` at the `SIGABRT`, and read down to the first frame in `src/`: `log_task (arg=...) at src/log.c:58`, just below `xQueueReceive (xQueue=0x0, ...)`. That `0x0` is the bug. Note that the board's `hw:` report is missing: `abort()` skips it.

### In the wild

- Every vendor SDK (STM32Cube, NXP MCUXpresso, ESP-IDF) is these four ingredients plus a generated `FreeRTOSConfig.h`. Knowing which file is which is how you upgrade the kernel without breaking the product.
- Teams run their task logic on the POSIX port in CI, under gdb and sanitizers, before it reaches a board. The official FreeRTOS demos include a POSIX one for exactly this.
- On hardware, the same gdb commands work through a debug probe (OpenOCD or J-Link), and `configASSERT` with a breakpoint in the assert hook is the first thing seasoned engineers turn on.
