### Linux, 2006: finding deadlocks that haven't happened yet

For years, some of the worst Linux bug reports read like this: *"the machine froze once last week, no crash message, no log, can't reproduce it."* Very often the cause was two locks taken in opposite orders on two different code paths. That only hangs when both paths run at exactly the wrong moment, so it might happen once a month on one machine in a thousand.

In 2006 Ingo Molnar merged **lockdep** into Linux 2.6.18. It doesn't wait for the hang. It records the *order* in which every lock is taken, and the first time it sees B taken while holding A on one path and A while holding B on another, it reports a possible deadlock, even though that run didn't hang. It straight away started reporting lock-order bugs that had been sitting in the kernel for years.

The lesson for firmware: **you can't test your way out of a deadlock.** You have to make it impossible by design.

### Two tasks, two locks, one hang

You met mutexes in the last node. Now give a system two of them and two tasks that each need both:

```c
// nav (prio 2)                     // archive (prio 1)
lock(gps);                          lock(sd);
read_fix();                         open_file();
lock(sd);     // waits for archive  lock(gps);    // waits for nav
append_track();                     stamp_time();
unlock(sd); unlock(gps);            unlock(gps); unlock(sd);
```

```
tick          0   1   2   3   4   5 ...
archive (1)   S   .   .   s   W        S = locks sd, s = still working, W = waits for gps
nav     (2)       G   g   W            G = locks gps, g = still working, W = waits for sd
                              ^ both blocked forever
```

archive takes `sd`. nav wakes, preempts it and takes `gps`. Now nav wants `sd` (archive has it) and archive wants `gps` (nav has it). Nobody can move, and nothing will ever change that. If nav had woken two ticks later, archive would have finished its round and everything would have worked. That's why deadlocks pass bench testing: **they need a specific interleaving**, and the bench rarely produces it.

### The four conditions (Coffman, 1971)

A deadlock needs **all four** of these at once:

| condition | meaning | in our example |
|---|---|---|
| **mutual exclusion** | a resource can be held by only one task | a mutex, by definition |
| **hold and wait** | a task holds one resource while waiting for another | nav holds gps and waits for sd |
| **no preemption** | nobody can take a resource away from its holder | the kernel never steals a mutex |
| **circular wait** | a cycle: A waits for B, B waits for A (or a longer ring) | nav → sd → archive → gps → nav |

Break **any one** and deadlock becomes impossible. You usually can't give up mutual exclusion (that's why you have the lock), and the kernel won't preempt mutexes for you. So in practice you attack the other two.

### Fix 1: one global lock order (breaks circular wait)

Number your locks and **always take them in increasing order**, everywhere. If every task takes `gps` before `sd`, then a task holding `sd` never waits for `gps`, so a cycle can't form:

```c
/* LOCK ORDER: gps_lock -> sd_lock. Never take gps_lock while holding sd_lock. */
lock(gps);  lock(sd);
...
unlock(sd); unlock(gps);   // release order doesn't matter for deadlock, reverse is tidy
```

Now one task simply waits its turn. This is the standard fix, it costs nothing at run time, and it works for any number of locks. The hard part is discipline: **write the order down** next to the lock declarations, because the next deadlock will come from someone who didn't know it.

The sneaky version is a **hidden** lock: `nav` holds `gps` and calls `log_msg()`, which takes `sd` deep inside. The order is broken without anyone seeing two `lock()` calls next to each other. Know what the functions you call while holding a lock will lock.

### Fix 2: timeout and back off (breaks hold and wait)

When you can't impose an order (say, a library takes its own locks), don't wait forever. Wait a bounded time, and if you don't get the second lock, **release everything you hold**, back off, and retry:

```c
for (;;) {
    rtos_mutex_lock(sd_lock, RTOS_WAIT_FOREVER);
    if (rtos_mutex_lock(gps_lock, 5)) break;   // got both
    rtos_mutex_unlock(sd_lock);                // give back what I hold
    rtos_delay(backoff);                       // let the other task finish
}
```

This works, but it has costs. Work done before the failure may have to be redone. And if two tasks back off for the *same* time, they can retry in lock-step forever: **livelock**, where everyone is busy and nobody makes progress. Use different (or random) back-offs. Lock ordering is simpler; reach for this when you can't.

A timeout is also a great **detector**. `rtos_mutex_lock(m, 100)` returning `false` in a system where the lock is normally held for 2 ticks means something is badly wrong: log it, count it, assert in debug builds.

### Other ways out

- **Don't nest.** If one mutex protects both the GPS data and the SD card, there's nothing to order. Coarser locks cost some concurrency, which on one small core is often nothing.
- **Copy, then release.** nav could lock `gps`, copy the fix into a local variable, unlock, and *then* lock `sd` to write it. Holding one lock at a time can't deadlock.
- **Hand it to one task.** Make a single "storage" task the only one that touches the SD card, and send it work through a queue. No shared card, no card lock.
- **Priority ceiling.** With the immediate priority ceiling protocol on a single core (OSEK/AUTOSAR use it), a task that holds a mutex runs at the highest priority of anything that uses it, so nobody who could form a cycle gets to run. As long as tasks don't block while holding a lock, deadlock between those mutexes can't happen.

### Gotchas

- **Priority inheritance doesn't help.** It shortens waits; it can't break a cycle. Both tasks still wait forever, just at a higher priority.
- **Adding a delay is not a fix.** `rtos_delay(1)` between the two locks changes the timing so *this* test passes. The cycle is still possible. The lab tests several start-up timings for this reason.
- **Self-deadlock.** Locking a non-recursive mutex you already hold deadlocks with only one task. The simulator stops with an error. FreeRTOS has a separate recursive mutex for code that needs this, and Zephyr's `k_mutex` is always recursive.
- **Not just mutexes.** Two tasks that each block forever sending to a full queue the other one should drain is the same cycle. So is a task that holds a mutex while waiting for a semaphore that only another task, which needs that mutex, will give.
- **Partial deadlock is the usual kind.** The simulator's `sim_deadlocked()` reports when *every* task is blocked forever and the CPU sits idle with nothing left to wake it. In real firmware, the LED task, the idle hook and the UART keep running while two tasks are frozen. A watchdog that's kicked from one healthy task won't notice. Make every important task check in, and kick the watchdog only when all of them have.

### In the wild

- **Linux lockdep** (`CONFIG_PROVE_LOCKING`) is still the gold standard: it proves lock-order violations from a single run where nothing hung. **ThreadSanitizer** reports "lock-order-inversion (potential deadlock)" the same way for pthread programs.
- **FreeRTOS** has no deadlock detector. Teams use bounded timeouts on `xSemaphoreTake(m, pdMS_TO_TICKS(50))` (it returns `pdFALSE` on timeout) together with `configASSERT`. **Zephyr's** `k_mutex_lock` returns `-EAGAIN` on timeout.
- **POSIX:** a `PTHREAD_MUTEX_ERRORCHECK` mutex returns `EDEADLK` instead of hanging when a thread relocks it.
- **Databases** live with deadlocks: PostgreSQL notices the cycle after `deadlock_timeout` (1 s by default) and aborts one transaction so the others can continue. Firmware rarely has the luxury of a victim, which is why we design deadlocks out.
- **Watchdogs with per-task check-ins** are the last line of defence in shipped products. They turn "froze mid-ride until the battery died" into "rebooted in a second, and logged which task stopped checking in".
