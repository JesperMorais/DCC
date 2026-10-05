### Every story was one small mistake

Think back over this section's stories. Mars Pathfinder kept resetting because one mutex had priority inheritance switched off. The Patriot battery's clock drifted by a third of a second because a tiny rounding error was never corrected. On the Therac-25, two pieces of code disagreed about shared state when an operator typed fast. Spirit rebooted in a loop because a table in RAM grew until it ran out.

None of these were exotic. Each one was a **single fundamental, slightly wrong, inside a system that otherwise worked**. That's what makes firmware hard. The concepts are simple one at a time, but in a real product they all happen at once, on the same tick, and they all share the same CPU.

This boss is that "all at once" moment.

### One system, every idea

Here's the shape of almost every sensor product: a fridge monitor, a smart meter, a wearable, an industrial gateway.

```
  hardware            interrupt            tasks, by priority                      outputs
 ─────────          ───────────          ─────────────────────────────────        ──────────
 UART bytes ──IRQ──▶ ISR: read DR, ack ──queue──▶ parser (FSM) ──lock──▶ shared state
                     push, never wait                                      │    │
 tick ──────────────────────────────────▶ alarm, every 100 ms ◀───────────┘    ├──▶ GPIO pin
                                          uplink, every 1 s ◀──────────────────┘──▶ modem
```

Each arrow is a lesson you've already done:

| Layer | The rule | Node |
|---|---|---|
| Registers | read-modify-write, change only your bits | bits, MMIO |
| ISR | do the minimum, acknowledge, never block | interrupts |
| ISR → task | copy into a queue with no wait, count drops | queues |
| Parsing | an explicit state machine, one byte per step | state machines |
| Who runs first | priority by **deadline**, not importance | tasks, priorities |
| Shared data | one owner, or a lock held for zero-ish time | races, mutexes |
| Locks | a mutex with inheritance, in one global order | mutexes, deadlock |
| Periodic work | `delay_until` on a fixed grid, never `delay` | timing |

### Worked example: one byte's journey

A temperature frame lands on the UART at tick 235. Here's every place it could go wrong:

1. **The ISR** reads `DR`, clears `RXNE` and copies the byte into the queue with `RTOS_NO_WAIT`. If it waited, or called `rtos_busy`, it would hold up every other interrupt. If it forgot to clear `RXNE`, the next byte would set `ORE` and be lost.
2. **The parser** wakes in the same tick, because it's the highest-priority task that's ready. Its state machine steps through `SYNC → MSB → LSB → CHK`. A data byte that happens to equal the sync byte is *still data*, because the state, not the byte value, decides what a byte means.
3. **It needs the lock**, but the low-priority EEPROM writer holds it, and a medium-priority screen redraw has just become ready. That's the Pathfinder arrangement exactly. With a plain semaphore, the redraw runs for 70 ms while the parser waits. With a **mutex with priority inheritance**, the EEPROM writer borrows the parser's priority, finishes its 3 ms and lets go.
4. **The alarm** reads the stored value at its next 100 ms slot. It does 2 ms of work every cycle, so it has to sleep with `delay_until`, or its checks slide 2 ms later every time.
5. **The uplink** copies the numbers under the lock and then spends 40 ms on the modem *outside* it. If it held the lock while transmitting, it would block the parser for 40 ms, and inheritance would boost a background task up to the parser's priority to finish the job.

### Choosing priorities: deadline, not importance

It's tempting to give the "most important" job the highest priority. A better guide is **how soon each job must finish once it's triggered** (deadline-monotonic scheduling):

- The alarm check must start *exactly* on its tick and takes 2 ms, so it has the tightest deadline.
- The parser has slack: the queue holds two frames, so it can fall up to about 50 ms behind before bytes are lost. But it must never sit behind a 70 ms redraw.
- The redraw and the uplink can wait.

The parser runs *more often* than the alarm, yet it goes below it. Rate-monotonic ordering (shorter period gets higher priority) is the classic starting point, and it's right whenever the deadline equals the period. When a queue gives a task slack, or a job must start with zero jitter, order by deadline instead.

### Gotchas when it all comes together

- **A fix in one place moves a bug somewhere else.** Raise the parser above the alarm and the parsing is perfect, but its 1 ms of calibration now lands on the alarm's tick. Check the whole timeline, not just the part you changed.
- **Slow work under a lock is the most common inversion.** Inheritance limits the damage, but it doesn't remove it. The real fix is a shorter critical section: copy under the lock, then do the slow part outside.
- **A snapshot and its reset belong in the same lock hold.** Read the window's min/max, unlock, then lock again to reset them, and a frame that arrives in between vanishes from both reports.
- **Signed data needs care.** `(MSB << 8) | LSB` is an `int`. Convert it to `int16_t` on purpose, or -4.0 °C (`FF D8`) is read as 65496, which is 6549.6 °C.
- **Every drop needs a counter.** Queue full, bad checksum, overrun: if you count them, a field problem becomes a number you can read.

### In the wild

- **Cold-chain monitors, smart meters and infusion pumps** are this exact architecture: a sensor ISR, a parser task, a periodic safety check that drives an output, and a slow radio link that never gets to hold a lock.
- **FreeRTOS** spells it `xQueueSendFromISR` + `portYIELD_FROM_ISR`, `xSemaphoreCreateMutex` (with inheritance) and `vTaskDelayUntil`. **Zephyr** spells it `k_msgq_put(..., K_NO_WAIT)`, `k_mutex` and periodic `k_timer`s. **Embedded Linux** uses `epoll` on a UART file descriptor, `pthread` mutexes with `PTHREAD_PRIO_INHERIT`, and `timerfd`. Those are the three branches this boss unlocks, and each one maps straight back to the table above.
- **Safety standards** such as IEC 62304 (medical software) and IEC 61508 (functional safety) ask you to justify your priority assignment and to show that shared data is protected. The table above is the start of that argument.
