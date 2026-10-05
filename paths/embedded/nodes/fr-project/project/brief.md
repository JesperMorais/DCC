### The sump under the pump house

A pump house at the bottom of a hill collects groundwater in a concrete sump. Water seeps in all the time. A pump drains it, and a small board with a microcontroller decides when the pump runs. If the pump never starts, the sump overflows and floods the electrics. If it never stops, it runs dry and burns out its seals in minutes. The controller that's there today is a `while(1)` loop with `delay()` calls, and it froze last winter when a sensor cable came loose. The replacement runs FreeRTOS.

Until now you've used a simulator that speaks FreeRTOS's API. This time it's **the real kernel**: the actual FreeRTOS-Kernel sources (`tasks.c`, `queue.c`, `timers.c`), pinned to a release tag, compiled on your own Linux machine with the official **POSIX port**. Each task is a real pthread, the tick is a real signal, and `make` builds a real program you can run, debug with `gdb` and break. The board itself (the sump, the level sensor, the pump and an emergency-stop button) is simulated in `hw.c`, and it reports the truth when the program exits: did the tank overflow, did the pump run dry, did the pump keep running after someone hit the e-stop?

You'll build the controller in four steps:

- a sensor task that samples every 100 ms **without drifting**, even though each conversion takes a different time, and a logger task that is the only one allowed to print;
- a control task fed by a **queue**, running the pump with hysteresis and keeping a **mutex-protected** status;
- an **e-stop interrupt** with a bouncing button, handled with `FromISR` APIs, a **task notification** and `portYIELD_FROM_ISR`, so the pump stops in the same tick;
- a **software timer** that gives a heartbeat and trips a **watchdog** when the sensor dies.

This is the whole FreeRTOS branch in one program. The data types and the kernel configuration are given; the tasks, their priorities and the structure are yours. The tests run your program with scenarios like "press the e-stop at tick 1537" and check what it printed and what the board measured, so any sound design passes.

**How to start:** copy the starter (the command is on this page) and run `make build`. The first build clones the kernel into `third_party/`. Read `README.md`, especially the section on what's different on the POSIX port, then `src/hw.h` like a datasheet. Then start milestone 1 and check it with `make test-m1`.
