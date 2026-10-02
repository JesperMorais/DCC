In 1991, if you ran Windows 3.1 and one program got stuck in a loop, *the whole computer* froze. The mouse stopped and the clock stopped. Windows 3.1 was **cooperative**: every program had to hand the CPU back voluntarily, and one that didn't held everyone hostage. Classic Mac OS worked the same way until 2001.

Your superloop is Windows 3.1. Every job must return quickly, and one slow job delays all the others. Time to fix that the way the rest of the industry did.

### Where the superloop runs out

The FSM approach from the last node works beautifully until:
- **One job is long.** Writing a 4 KB flash page takes 20 ms, and an FFT takes 15 ms. You can chop it into FSM states by hand ("erase sector, step 3 of 12…"), but the code turns into spaghetti.
- **Deadlines differ.** The motor control loop needs to run every 1 ms *exactly*, while the display is happy with 50 ms. In a superloop the motor waits behind the display, because worst-case latency is the *sum* of all the jobs.
- **Waiting is everywhere.** "Wait for the radio's ACK, up to 100 ms" becomes yet another state machine.

### Tasks: many `main()`s, one CPU

An RTOS lets you write each job as its own infinite loop, a **task**, with its own stack, as if it owned the CPU:

```c
void control_task(void *arg) {
    for (;;) {
        read_sensors_and_drive_motor();   // 2 ms of work
        rtos_delay(8);                    // sleep: hand the CPU to someone else
    }
}
rtos_task_create("control", control_task, NULL, 3);   // priority 3 = most urgent
```

The **scheduler** decides which task runs. In a fixed-priority **preemptive** RTOS the rule is brutally simple:

> The highest-priority task that is ready to run is the one running. Always.

If the logger (priority 1) is halfway through writing a file when the control task's delay expires, the kernel **preempts** the logger *right then*, runs control, and resumes the logger exactly where it stopped. The logger never notices.

### Context switches

How do you "resume exactly where it stopped"? Each task has its own stack and a **TCB** (task control block). A context switch:

```
tick interrupt ─▶ save logger's registers onto logger's stack, SP → logger TCB
              ─▶ scheduler: control is ready and outranks logger
              ─▶ load control's SP from its TCB, pop its registers
              ─▶ return from interrupt … into control_task
```

On a Cortex-M this takes about 1–2 µs. It costs something, but it's cheap compared with the latency you'd otherwise have.

### Task states

```
            create
              │                 delay / wait for something
              ▼             ┌────────────────────────────┐
  ┌──────▶ READY ──scheduled──▶ RUNNING ──────────────▶ BLOCKED
  │           ▲  ◀─preempted──┘                            │
  └───────────┴──────────── time's up / event arrived ─────┘
```

- **Running**: on the CPU right now (one task, on a single core).
- **Ready**: wants the CPU, but someone more important has it.
- **Blocked**: waiting for time (`rtos_delay`) or an event (semaphore, queue). It uses **zero** CPU and isn't even considered by the scheduler.

When *no* task is ready, the CPU runs the **idle** task, which on real hardware executes `WFI` and sleeps. That's where a battery product gets its battery life.

### Why blocking beats busy-waiting

```c
while (!flash_ready) { }            // ✗ busy-wait: RUNNING the whole time
while (!flash_ready) rtos_delay(1); // ✓ BLOCKED, checks once per tick
```

A busy-waiting task is *running*. At high priority it starves everything below it, so a busy-waiting logger at priority 3 means your motor loop never runs. At low priority it still eats every spare cycle, so the CPU never idles, never sleeps, and your CPU-load metric reads 100% when the real load is 30%. (In *Semaphores* you'll replace even the 1-tick poll with "wake me when it happens".)

### Play with it

Here are three tasks: a fast control loop, a blinker, and a logger with lots of work. Watch the logger's job get **sliced up** each time control preempts it.

```playground
{
  "title": "Preemption: control slices through the logger",
  "ticks": 80,
  "editable": true,
  "tasks": [
    { "name": "control", "priority": 3, "period": 10, "wcet": 2 },
    { "name": "blink", "priority": 2, "period": 40, "wcet": 3, "offset": 5 },
    { "name": "logger", "priority": 1, "period": 40, "wcet": 14 }
  ]
}
```

Challenges:
1. Give `logger` priority **4**. Does `control` still make every 10-tick deadline?
2. Put the priorities back and raise the logger's `wcet` until it misses *its own* deadline. What's the CPU utilisation when that happens?

### Gotchas

- **Priorities are a design decision, not a ranking of importance.** The logger may matter most to the business, but it has the loosest deadline, so it gets the lowest priority. Short deadline means high priority. (*Priorities & deadlines* makes this precise.)
- **Every task needs a blocking point.** A task that never delays or waits is a busy-wait, and nothing below it ever runs.
- **`rtos_yield()` isn't blocking.** The task stays *Ready*, so if nothing else is ready it's scheduled again at once, and a `while (!flag) rtos_yield();` loop spins without time ever passing. Block with a delay or a wait instead.
- **Each task has its own stack.** Ten tasks with 1 KB each is 10 KB of RAM. Sizing stacks is a real job.
- **`rtos_delay(8)` after 2 ticks of work drifts** if the work varies. *Ticks, delays & periodic tasks* fixes that with `rtos_delay_until`.

### In the wild

- **FreeRTOS** (`xTaskCreate`, `vTaskDelay`) runs in billions of devices. Its scheduler is the exact rule above.
- **Zephyr** threads, **ThreadX** (Azure RTOS) and **VxWorks** (Mars rovers, the Boeing 787) are all fixed-priority preemptive at heart.
- **Linux `SCHED_FIFO`** is the same policy for real-time threads.
- That Windows 3.1 freeze is why every modern desktop OS is preemptive.

In the lab you'll take over an RTOS port someone else started. The priorities are wrong and the logger busy-waits, so the control loop starves. You'll fix both and then read the story in the Timeline.
