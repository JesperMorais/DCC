## The badge reader that missed people

An office door controller (Zephyr on an STM32) reads RFID badges. The reader chip raises a **"data ready" interrupt** (`card_isr`) for every badge it sees. The `reader` thread (priority 2) decrypts the badge (2 ms of CPU) and records it in a shared `door_log`. Two given threads also touch the system:

- `cloud` (priority 10) uploads the log every 50 ms and **holds the log lock for 5 ms** while its slow modem talks.
- `ui` (priority 6) redraws the e-ink display: 20 ms of CPU, starting at 52 ms and every 100 ms after that.

Facilities reports that people "sometimes have to badge twice", the "reader offline" alarm never fires, and the door sometimes freezes for good. Fix it.

### Requirements

1. **ISR → semaphore → thread.** `card_isr` must not do the work and must not block. It signals `card_sem`, and the reader starts on the **same tick** as the interrupt (no polling).
2. **Every badge counts.** Three badges 1 ms apart (each takes 2 ms to process) must give `door_log.cards == 3`.
3. **Heartbeat timeouts.** The reader waits at most `READER_TIMEOUT_MS` (100 ms). If `k_sem_take` returns **`-EAGAIN`**, that is *not* a badge: call `log_timeout()`. After 350 ms of silence: `cards == 0`, `timeouts == 3`.
4. **The timeout window restarts after each badge.** A badge at 60 ms means no timeout at 100 ms.
5. **Shared log under a real `k_mutex`.** `log_card()` and `log_timeout()` lock the log and then call `log_touch()`, which locks it **again**. That's fine with a Zephyr mutex, because it's recursive. `door_log.updates` must match the number of changes, and nothing may deadlock.
6. **No priority inversion.** A badge at 51 ms (while `cloud` holds the lock and `ui` wakes at 52) must be fully processed by 60 ms. With a mutex, `cloud` inherits the reader's priority until it unlocks.

Keep `card_isr`, the thread names and the given threads' timing. Then look at the **Timeline** for the last test: at tick 53 there's a priority event on `cloud`: it is borrowing the reader's priority, and it hands it back when it unlocks at 57. (The timeline draws priorities on the simulator's internal scale, where a bigger number is more urgent, so you'll see `90 -> 98` there. That's Zephyr's 10 -> 2.)
