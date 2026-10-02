### Mars, again, and why Zephyr made the fix the default

In 1997 the Pathfinder lander kept resetting on Mars. A low-priority meteorology task held a mutex, a medium-priority communications task kept it off the CPU, and the high-priority bus task waited until the watchdog fired. JPL fixed it by **turning on priority inheritance** remotely. You rebuilt that bug in Fundamentals. Zephyr's answer is simple: every `k_mutex` has priority inheritance, always, and you can't turn it off.

That leaves one decision for you: **semaphore or mutex?** They look similar, and picking the wrong one is the most common Zephyr sync bug.

### k_sem: counting events

A semaphore is a counter of *things that happened*. Anyone can give, including an ISR, and threads take.

```c
K_SEM_DEFINE(rx_sem, 0, 16);   /* initial count 0, limit 16 */

void uart_isr(void) {          /* ISR context: never blocks */
    k_sem_give(&rx_sem);
}

void rx_thread(void *a, void *b, void *c) {
    for (;;) {
        int rc = k_sem_take(&rx_sem, K_MSEC(100));
        if (rc == -EAGAIN) {   /* 100 ms of silence */
            report_link_down();
            continue;
        }
        handle_byte();         /* rc == 0: we got one */
    }
}
```

This is the ISR-to-task pattern you built in Fundamentals with `rtos_sem_give`, and it works the same way. Zephyr adds three details worth knowing:

- **The limit matters.** `K_SEM_DEFINE(s, 0, 1)` is a *binary* semaphore. Three gives before one take leave a count of 1, and two events are gone. If every event must be counted, give it room (or use `K_SEM_MAX_LIMIT`).
- **The return code tells you why you woke up.** `k_sem_take` returns **0** (got it), **-EBUSY** (you passed `K_NO_WAIT` and it was empty), or **-EAGAIN** (the timeout expired). Code that ignores the return value treats silence as an event. That's a real and common bug.
- **`k_sem_give` is ISR-safe, and `k_sem_take` from an ISR is only allowed with `K_NO_WAIT`.** An ISR can never wait.

### k_mutex: owning a resource

A mutex protects *a thing* (a struct, a bus, a file) and has an **owner**: the thread that locked it. That ownership buys three features a semaphore can't offer:

1. **Priority inheritance.** If a more urgent thread blocks on a mutex, the owner temporarily runs at the waiter's priority until it unlocks. Medium-priority threads can't wedge themselves in between. (The Pathfinder fix, built in.)
2. **Recursion.** The owner can lock it again. Zephyr counts the locks, and the mutex is released when the count goes back to zero. A helper that locks can be called from a function that already holds the lock:

```c
K_MUTEX_DEFINE(cfg_lock);

static void cfg_touch(void) {          /* callable with or without cfg_lock */
    k_mutex_lock(&cfg_lock, K_FOREVER);
    cfg.version++;
    k_mutex_unlock(&cfg_lock);
}

void cfg_set_rate(int hz) {
    k_mutex_lock(&cfg_lock, K_FOREVER);
    cfg.rate = hz;
    cfg_touch();                       /* nested lock: fine */
    k_mutex_unlock(&cfg_lock);
}
```

3. **Ownership checks.** `k_mutex_unlock` from a thread that doesn't own it returns **-EPERM**, and unlocking a mutex that isn't locked returns **-EINVAL**.

The price is that **mutexes are for threads only**. An ISR can't own anything and can't wait, so `k_mutex_lock` in an ISR is a bug. ISR-side shared data needs `irq_lock()`/`irq_unlock()`, atomics, or a hand-off through a semaphore or queue.

### "A semaphore initialised to 1 is a lock, right?"

It *works*, right up until it doesn't:

| | `K_SEM_DEFINE(l, 1, 1)` used as a lock | `K_MUTEX_DEFINE(l)` |
|---|---|---|
| Priority inheritance | no: Pathfinder inversion is possible | yes, always |
| Lock twice from the same thread | **deadlocks itself** | fine (recursive) |
| Wrong thread unlocks | silently allowed | `-EPERM` |
| Usable from an ISR | give only | no |

Rule of thumb: **signalling an event** means `k_sem`. **Protecting a resource** means `k_mutex`.

### Worked example: watching inheritance happen

`cloud` (priority 10) locks the door log and starts a 5 ms upload. At tick 51 a badge interrupt wakes `reader` (priority 2). The reader decrypts for 2 ms, then needs the log, which `cloud` still holds. At 52 the `ui` thread (6) wakes up with 20 ms of drawing to do.

```
tick     50 51 52 53 54 55 56 57 58 ... 76
cloud    #  .  .  #  #  #  #  .            prio 10 -> 2 at 53, back to 10 at 57
reader      #  #  b  b  b  b  ok           b = blocked on log_lock, ok = badge logged
ui             r  r  r  r  r  #  #  ...  #  r = ready, but outranked
```

Without inheritance, `ui` (6) would beat `cloud` (10) and the reader would wait until ~77 ms. With `k_mutex`, `cloud` borrows priority 2 at tick 53, finishes its upload, unlocks at 57, and drops back to 10. The reader is done 20 ms sooner.

### Gotchas

- **Keep critical sections short.** Inheritance limits the *extra* waiting, but the reader still waits for the owner's whole critical section. Don't do 5 ms SPI transfers under a lock if you can copy and release.
- **Lock ordering still matters.** Inheritance doesn't prevent deadlock. Take locks in a fixed global order, as in Fundamentals.
- **Use `K_FOREVER` deliberately.** A timeout on `k_mutex_lock` that returns -EAGAIN usually means a design bug, so log it loudly instead of retrying silently.
- **Don't give a semaphore "to unlock" in a different thread than the one that took it** unless that hand-off is the whole point (a signalling pattern).

### In the wild

- **Sensor and bus drivers** in Zephyr (`i2c_transfer`, SPI with `spi_context`) use a `k_sem` internally for "transfer complete" from the ISR, and a lock to serialise callers of the bus.
- **Nordic's nRF Connect SDK samples** use `k_sem` with a timeout as a liveness check, e.g. "no GNSS fix within N seconds" on the nRF9160.
- **Industrial STM32 nodes** usually wrap shared config structs in a `k_mutex` with tiny critical sections: copy under the lock, act outside it.
