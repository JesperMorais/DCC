### The hub that only failed when someone was watching

The vibration hub passed a week on the bench. On site, it bolted onto a pump and paired with the plant's gateway, and the dashboard started showing gaps: a few IMU samples missing every few seconds, and reports that arrived at 101 ms, 201 ms, 302 ms. Nothing crashed and nothing asserted. The bench unit never had a phone connected over BLE, so the 30 ms radio bursts that exposed every scheduling mistake simply never happened there. This boss is that hub. Every Zephyr tool in this branch goes into one image, and the bugs only show up when everything runs at once.

### The architecture

```
 imu_isr (every 4 ms) ──k_msgq──▶ processing ──k_mutex──▶ stats
                                                           ▲
 report_timer (100 ms) ──k_work_submit──▶ sysworkq (-1) ───┤
 logger (legacy: holds the lock 5 ms, every 50 ms) ────────┘
 ble: 30 ms of CPU every 100 ms
```

Each box is a lesson you've done:

| Box | Lesson | The rule it brings |
|---|---|---|
| `imu_isr` → `imu_q` | `z-msgq` | An ISR puts with `K_NO_WAIT` and counts the drops. It never waits, and it never takes a mutex. |
| `processing` | `z-threads` | A thread with `K_THREAD_DEFINE` that blocks on `k_msgq_get(..., K_FOREVER)` and uses no CPU while it waits. |
| `stats_lock` | `z-sync` | `k_mutex`, not `k_sem`, because only the mutex has priority inheritance. |
| `report_timer` | `z-timers` | The expiry function runs in **ISR context**, from the system clock interrupt. |
| `report_work` | `z-work` | The ISR-to-thread handoff that Zephyr gives you for free: the system workqueue, priority -1, **cooperative**. |

In a real product, `z-devicetree` is in there too: the IMU's SPI bus and interrupt line come from the board's devicetree. The simulator calls `imu_isr` for you.

### Four design decisions to get right

**1. The timer fires in an ISR, so hand the work off.** A `k_timer` expiry function looks like a callback, but it runs inside the system clock interrupt. Taking a mutex there is illegal: on real Zephyr, with asserts enabled, it fails at once. Spending 2 ms formatting a report there blocks every other interrupt for 2 ms. So the expiry does one thing, `k_work_submit()`, and the work handler, which runs in a real thread, does the rest. This is the same "do the minimum in the ISR" rule you used for the IMU, applied to a timer.

**2. Copy under the lock, send outside it.** The report needs `processed` and `sum` from the **same moment**. Read them without the lock, and the reporter can catch `processing` between `processed++` and `sum += value`: the counts no longer match and the gateway computes a wrong mean. So take the lock, copy the whole struct, and unlock. The 2 ms send happens on the copy, with the lock free. Holding the lock across the send would make `processing` wait 2 ms every report. Inheritance doesn't shorten a critical section. It only stops other threads from making it longer.

**3. Pick priorities by asking who must never wait for whom.** Remember the Zephyr rule: a lower number is more urgent, and a negative one is cooperative. Then ask two questions:

- *Who must never wait behind a 30 ms BLE burst?* A burst covers about 7 IMU messages, and the queue holds 4. Whoever drains the queue has to beat `ble`.
- *Who must be able to interrupt whom?* The report work runs on the system workqueue at -1. It can preempt a **preemptible** thread the instant it's submitted, but it can't preempt a cooperative thread that's busy. If `processing` is cooperative and is 0.4 ms into a 1 ms filter when the timer fires, the report starts when the filter ends, and lands late.

"Make the important thread cooperative" feels safe, and it's the trap. Cooperative means *nothing* preempts you, including the kernel's own workqueue, Bluetooth's host stack and every driver that defers work to it.

**4. Let the mutex deal with the legacy code.** The logger is old code you're not allowed to touch, and it does its 5 ms flash write while holding `stats_lock`. You don't fix that by making the logger urgent. You fix it by making sure that, when it holds the lock, it's lent the priority of whoever is waiting. A `k_mutex` does that for you.

### Worked example: t = 50 to 56 ms

This is the moment the tests are built around. Watch it in the Timeline once your code works.

1. **t = 50:** `logger` wakes and takes `stats_lock`. It needs 5 ms of CPU to finish the write.
2. **t = 51:** `ble` starts its first 30 ms burst. It's more urgent than the logger, so it preempts it. The logger is now **holding the lock and not running**.
3. **t = 52:** the IMU interrupt fires. `processing` wakes, filters for 1 ms, and calls `k_mutex_lock()`. The lock is held, so it blocks.

What happens next is the whole difference between the two lock types:

- **With a `k_sem` as the lock:** the logger stays at its own low priority, so `ble` keeps the CPU until about t = 82. The logger finishes its write around 86, and only then does `processing` get the lock. That's more than 30 ms of no processing, while the ISR keeps adding a message every 4 ms to a 4-slot queue. Drops. This is Mars Pathfinder, on your desk.
- **With a `k_mutex`:** at t = 53 the kernel raises the logger to `processing`'s priority. It preempts `ble`, finishes its last few ms of the write, and unlocks around 57. Its priority drops back down, `processing` takes the lock and catches up on the queue. The worst latency stays under 10 ms, and `ble` loses only a few ms of its burst.

In the Timeline, look for `logger` running in the middle of a `ble` burst. That's inheritance, and it's what you want to see.

### Gotchas

- **`k_uptime_get_32()` in the ISR, `k_uptime_get()` in the report.** The message's timestamp is when the sample was taken, not when it was processed. Latency is the difference between the two.
- **`isr_produced` and `isr_drops` live outside `stats`** because the ISR can't take the lock. Only the ISR writes them, so they need none.
- **Start the timer once**, in `hub_main`, with both duration and period set to 100 ms. A one-shot timer restarted from the handler drifts by however late the handler ran.
- **Don't change the given threads.** Working around the logger by editing it is the one fix the project doesn't allow, and the one real legacy code rarely allows either.

### In the wild

- This is the skeleton of Zephyr's own **sensor subsystem**: a data-ready interrupt (a *trigger*) that does almost nothing, a handler that runs in a thread or on the system workqueue (`CONFIG_<SENSOR>_TRIGGER_GLOBAL_THREAD`), and application code that consumes the samples. Condition-monitoring sensors on pumps and motors, and wearables doing step counting, all look like this.
- **Zephyr's Bluetooth host** runs its own work on the system workqueue and dedicated threads. If an application thread hogs the CPU or sits cooperative for too long, connection events slip and phones disconnect. That's why application threads are normally preemptible, and cooperative priorities are kept for short, performance-critical kernel and driver work.
- `k_mutex` priority inheritance is always on; you can't forget to enable it. `CONFIG_PRIORITY_CEILING` sets how far a holder can be raised, which matters when cooperative threads contend for a lock that preemptible ones also use.
- Review comments you've now earned the right to give: *"mutex in a timer expiry"*, *"why is this thread cooperative?"*, *"lock held across the UART write"*, *"k_sem used as a lock: no inheritance"*, *"two reads of shared stats without the lock"*, *"K_FOREVER in an ISR"*.
