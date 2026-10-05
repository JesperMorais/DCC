A robot arm's 1 kHz servo loop ran perfectly on the bench for a week. Then, at the trade show, someone plugged in a USB stick to copy a demo video, the loop missed by **4 ms**, and the arm jerked hard enough to knock over a coffee cup. Average latency was 8 µs. The worst case decided the outcome.

Real-time doesn't mean fast. It means **bounded**: the worst case is known and small enough. This node is about making Linux bounded, and about *measuring* that it is.

### Why stock Linux is non-deterministic

On your RTOS in Fundamentals, the highest-priority ready task always ran on the next tick. Linux was built for throughput, and it has places where it **can't** be preempted:

- **Interrupt handlers** (hard IRQs) and **softirqs** (network RX, block I/O completion, timers) run ahead of every thread, including your `SCHED_FIFO` priority-99 one. A USB storage storm is mostly softirq work.
- **Spinlocks** disable preemption while they're held. Any kernel code path holding one, maybe for a long list walk, delays you.
- **Page faults.** Touching a page of memory for the first time, or touching one that was swapped out, goes into the kernel and can take milliseconds (much longer if it hits disk).
- **SMIs, power management and caches:** firmware stealing the CPU (System Management Interrupts on x86), deep C-states that take hundreds of µs to wake from, and cache and TLB misses after another process ran.

### PREEMPT_RT: making the kernel preemptible

The **PREEMPT_RT** patch set, merged into mainline Linux in **6.12** (2024) after 20 years out of tree, attacks the first two problems:

- **Threaded IRQs.** Almost every interrupt handler becomes a kernel thread (`irq/45-eth0`) with a `SCHED_FIFO` priority, 50 by default. The hard IRQ only wakes that thread. Now your priority-80 control thread *outranks* the network driver. You've seen this pattern before: it's the Fundamentals ISR-gives-a-semaphore-to-a-task pattern, applied to the whole kernel.
- **Sleeping spinlocks.** `spinlock_t` becomes an `rt_mutex`: preemptible, and with **priority inheritance**. Only `raw_spinlock_t` still spins with preemption off, and it's used for a handful of truly short sections.
- Softirqs run in threads too, so they're schedulable rather than "whenever".

The result: worst-case scheduling latency drops from milliseconds to tens of microseconds on decent hardware.

### Measuring it: cyclictest

`cyclictest` is the standard tool. It's a thread that sleeps until an absolute time (your `timerfd`/`clock_nanosleep(TIMER_ABSTIME)` loop from the previous nodes), then records **how late it woke up**:

```
# cyclictest --mlockall --priority=80 --interval=200 --histogram=400 --duration=12h
T: 0 ( 1234) P:80 I:200 C:216000000 Min:  2 Act:  4 Avg:  5 Max:  38
```

```
latency (µs)  count
     3      51 233 004   ██████████████
     5     119 882 551   ████████████████████████████████
     8      38 102 220   ██████████
    12       6 781 002   ██
    20           1 140
    38               1   ← the number that matters
```

Read it like an engineer: **Max** is your bound, not Avg. The tail matters, so run for **hours, under load**: `stress-ng`, network floods, disk I/O, GPU work, USB plug/unplug. A max measured on an idle board for 10 seconds tells you nothing. If one sample sits far out, find it with `trace-cmd`/`ftrace` before you ship.

### Your process: lock it down

```c
mlockall(MCL_CURRENT | MCL_FUTURE);         /* no page faults later */
prefault_stack(64 * 1024);                  /* touch your stack once */
struct sched_param sp = { .sched_priority = 80 };
pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp);   /* needs CAP_SYS_NICE */
/* … allocate every buffer now, never malloc() in the loop … */
```

- **`mlockall`** pins every page in RAM, so your loop never takes a page fault. Allocate and touch everything **before** the loop. It needs `CAP_IPC_LOCK` or a big enough `RLIMIT_MEMLOCK` (`memlock` in limits.conf), and it fails with `ENOMEM`/`EPERM` otherwise, so check its return value.
- **CPU isolation:** `isolcpus=3 nohz_full=3 rcu_nocbs=3` on the kernel command line, or cpusets/cgroups at runtime, keep the scheduler and most housekeeping off core 3. Pin your RT thread there with `pthread_setaffinity_np`, and steer IRQs away with `/proc/irq/*/smp_affinity`.
- **`SCHED_FIFO`** runs the highest priority first, like your Fundamentals RTOS, but it **never time-slices** equal priorities: a FIFO thread runs until it blocks, yields or is preempted by a higher one. `SCHED_RR` adds the round-robin time slice (so it's the closer match to the Fundamentals scheduler), and `SCHED_DEADLINE` takes runtime/period budgets (EDF).

### Gotchas: SCHED_FIFO bites back

- **RT throttling.** By default, `sched_rt_runtime_us = 950000` out of `sched_rt_period_us = 1000000`: RT threads get at most **95 %** of each second. A spinning FIFO thread gets paused for 50 ms every second, so the system stays alive and your "real-time" loop stalls. That's the right default, but know it's there. Fix the loop rather than setting the limit to -1. (Since 6.12, a "fair server" deadline reservation does this job by default, with the same 50 ms per second, and it only pauses RT threads when normal tasks are actually waiting.)
- **A busy loop at priority 99 starves everything** below it, including the kernel threads and IRQ threads that your own I/O depends on. Use priorities deliberately: control at 80, IRQ threads at 50, logging as a normal thread.
- **Priority inversion is back, in userspace.** A plain `pthread_mutex` has **no** priority inheritance. Share one with a low-priority logger and the Fundamentals scenario returns:

```playground
{
  "title": "RT control thread vs a logger, plain mutex (no inheritance)",
  "ticks": 40,
  "editable": true,
  "inheritance": false,
  "tasks": [
    { "name": "control (FIFO 80)", "priority": 3, "period": 20, "wcet": 3, "offset": 1, "deadline": 8, "lock": { "at": 0, "len": 2 } },
    { "name": "telemetry (FIFO 50)", "priority": 2, "period": 40, "wcet": 10, "offset": 2 },
    { "name": "logger (FIFO 10)", "priority": 1, "period": 40, "wcet": 4, "offset": 0, "lock": { "at": 0, "len": 3 } }
  ]
}
```

At tick 1, `control` blocks on the mutex that `logger` holds. At tick 2, `telemetry` preempts `logger` and runs for 10 ticks, and the high-priority thread waits for a medium one it doesn't even share anything with, missing its deadline at tick 9. **Your turn:** flip `"inheritance"` to `true` (that's `PTHREAD_PRIO_INHERIT`) and watch `logger` borrow priority 3, release the lock at tick 3, and let `control` finish at tick 6.

```playground
{
  "title": "Same system with PTHREAD_PRIO_INHERIT",
  "ticks": 40,
  "editable": true,
  "inheritance": true,
  "tasks": [
    { "name": "control (FIFO 80)", "priority": 3, "period": 20, "wcet": 3, "offset": 1, "deadline": 8, "lock": { "at": 0, "len": 2 } },
    { "name": "telemetry (FIFO 50)", "priority": 2, "period": 40, "wcet": 10, "offset": 2 },
    { "name": "logger (FIFO 10)", "priority": 1, "period": 40, "wcet": 4, "offset": 0, "lock": { "at": 0, "len": 3 } }
  ]
}
```

Other places latency hides: `printf` to a slow console, `malloc`, `fsync`, and **any syscall that may sleep** inside the loop. Hand the data to a non-RT thread through a lock-free ring instead.

### In the wild

- **LinuxCNC, EtherCAT masters (IgH, SOEM), KUKA and Universal Robots controllers** run 1–4 kHz loops on PREEMPT_RT with isolated cores.
- **SpaceX's** flight software runs on Linux, and pro-audio stacks (JACK/PipeWire with RT kernels) live or die by tail latency.
- The **OSADL** QA farm publishes cyclictest results for dozens of boards, running continuously under load. It's a great reference for what "good" looks like (tens of µs).
- **Interview insight:** "Is Linux real-time?" The strong answer is "it can be *soft*/firm RT with bounded latency in the tens of µs on PREEMPT_RT, if you also mlockall, isolate a core, use PI mutexes, avoid syscalls that sleep in the loop, and **prove** it with hours of cyclictest under load. For hard guarantees in the single microseconds, put the loop on an MCU or an R-core and let Linux supervise."
