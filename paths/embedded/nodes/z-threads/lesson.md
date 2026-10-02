### The flip that bites every newcomer

Your first Zephyr code review. You've written `K_THREAD_DEFINE(control, ..., 14, ...)` because control is the most important thread and 14 is the biggest number. The reviewer comments: *"This is now the least important thread in the system."* Welcome to Zephyr, where **a lower number means more urgent**, and **negative numbers mean something else entirely**.

### The same tasks, new names and new numbers

In Fundamentals you created tasks with `rtos_task_create(name, fn, arg, prio)`, and the scheduler always ran the highest-priority ready task. Zephyr threads are the same idea, with two changes:

```
          more urgent ◀───────────────────────────────▶ less urgent
  -16 ... -2  -1  │  0   1   2  ...  14
  ── cooperative ──┼──── preemptible ────
```

1. **The scale is flipped.** Priority 2 beats priority 10. The system workqueue runs at -1, `main()` at 0, and app threads usually sit somewhere around 1–10.
2. **Negative priorities are cooperative.** A cooperative (coop) thread, once it's running, is **never preempted by another thread**, not even a more urgent one. It keeps the CPU until it *chooses* to give it up: it sleeps, blocks on a semaphore, mutex or queue, or calls `k_yield()`. Interrupts still run, but any thread they wake has to wait.

Preemptible threads (0 and up) behave exactly like your Fundamentals tasks: the moment a more urgent thread becomes ready, they're switched out.

### Two ways to make a thread

```c
/* Static: defined at compile time, started when the kernel boots. */
static void blink(void *p1, void *p2, void *p3) { ... }
K_THREAD_DEFINE(blinky, 1024,          /* name, stack bytes          */
                blink, NULL, NULL, NULL, /* entry + 3 arguments        */
                7, 0, 0);              /* priority, options, delay ms */

/* Dynamic: you provide the stack and the struct, and start it at runtime. */
K_THREAD_STACK_DEFINE(rx_stack, 2048);
static struct k_thread rx_thread;

k_tid_t tid = k_thread_create(&rx_thread, rx_stack, K_THREAD_STACK_SIZEOF(rx_stack),
                              rx_entry, &uart0, NULL, NULL,
                              K_PRIO_PREEMPT(5), 0, K_NO_WAIT);
```

Prefer `K_THREAD_DEFINE` for threads that live forever, which is most of them. It's declarative, it's visible to tooling, and the stack is sized at compile time. Use `k_thread_create` when you need to choose at runtime: a worker per connection, or a thread that's started only after a self-test passes. The last argument is a start delay, and so is the last `0` in `K_THREAD_DEFINE`. Either way, **every thread gets its own stack, and you size it.** There's no heap fallback. A stack that's too small corrupts memory silently unless you enable `CONFIG_STACK_SENTINEL` or hardware stack protection (`CONFIG_HW_STACK_PROTECTION`). The entry function takes three `void *` arguments, which saves you a struct for simple cases.

The helper macros make intent readable: `K_PRIO_COOP(x)` is `-(CONFIG_NUM_COOP_PRIORITIES - x)`, which is always negative. `K_PRIO_PREEMPT(x)` is just `x`.

### When should a thread be cooperative?

Cooperative is a sharp tool. Use it when:

- **A short sequence must not be interleaved** with other threads: a 3-step radio TX setup, or a multi-register update to a peripheral. Being coop gives you that atomicity against other threads *without* a lock. (Against ISRs you still need `irq_lock()`.)
- **The thread is short, bounded and frequent.** Zephyr's own system workqueue (-1) and the Bluetooth RX thread are coop. Their handlers are expected to be quick, and not being preempted halfway saves context switches and locking.

Never make something cooperative when:

- **It does long work.** A 25 ms flash erase in a coop thread is 25 ms in which *nothing else* runs, including your 1 kHz control loop.
- **It spins waiting for something.** A coop thread that polls a flag without sleeping can livelock the system, because the thread that would set the flag never gets the CPU.

```
 coop logger (-2) running a 25 ms flash write:
 tick   0         10        20   25
 logger ##########################
 control           ^ ready at 10 ........ runs at 25   (15 ms late)

 preemptible logger (10):
 logger ##########.##########.####
 control          #          #           (on time, every time)
```

### Worked example: a coop thread delays even a *first* run

Here's a subtle one. `boot` is coop (-1) and spends its first 5 ms initialising a radio. `ctl` is preemptible (0), and the first thing it does is `k_msleep(1)`. You'd expect `ctl` to wake at tick 1. It wakes at **6**. Both threads are ready at boot. `boot` is more urgent and coop, so `ctl` doesn't even get to *execute its first line* (the `k_msleep`) until `boot` sleeps at tick 5. Then it sleeps for 1 ms from *there*. Coop threads don't just delay wake-ups, they delay everything.

### Gotchas

- **`k_yield()` from a coop thread only lets equal-or-more-urgent threads run.** A preemptible thread at priority 5 still waits. To let *everyone* run, actually sleep (`k_msleep(1)`) or block on something.
- **"More urgent" is not "faster".** Making everything coop or very urgent gives you a superloop with extra steps. Rate-monotonic thinking still applies: the shorter the deadline, the more urgent.
- **Time slicing** (`CONFIG_TIMESLICING`) only rotates *equal-priority preemptible* threads. Coop threads never get sliced.
- **`main()` is a thread too**, at priority 0 by default (`CONFIG_MAIN_THREAD_PRIORITY`). When it returns, the kernel keeps running the other threads.

### In the wild

- **Bluetooth on nRF52/nRF53:** the host's RX and TX threads are cooperative by default (`CONFIG_BT_RX_PRIO`), so the stack's internal state machines never get interleaved. That's also why a Bluetooth callback that blocks or runs long breaks your connection.
- **Motor drives and power supplies** keep the control loop preemptible and very urgent (or in an ISR), and push everything slow (flash logging, shells, telemetry) down into high-numbered preemptible threads.
- **Shell and logging backends** (`CONFIG_SHELL_THREAD_PRIORITY`, `CONFIG_LOG_PROCESS_THREAD_PRIORITY`) default to *low urgency*, meaning a high number, so debug output never steals time from real work.
