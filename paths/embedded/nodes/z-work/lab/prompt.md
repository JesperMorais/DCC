## The remote that ate button presses

A Zephyr-based TV remote has two buttons. Each button's interrupt must stay tiny, so the real handling (debounce check, an I2C write to the LED driver, a BLE notification: **2 ms** per press) is deferred to the **system workqueue**. A preemptible `ui` thread (priority 5) animates the display non-stop.

Each button is a struct with its own work item **embedded** in it:

```c
struct button {
    struct k_work work;
    char name;                 /* 'A' or 'B' */
    volatile uint32_t presses; /* written only by the ISR */
    uint32_t handled;          /* written only by the work handler */
};
```

Users say rapid presses get lost, and pressing B sometimes does A's thing.

### Your job

- **`button_b_isr`**: count the press and submit B's work item, exactly like A.
- **`button_work_handler`**: **one** handler serves both buttons (`app_main` registers it for both). Use **`CONTAINER_OF`** to get from the `struct k_work *` to its `struct button`. Then, for **every** press that hasn't been handled yet: `log_event(b)`, `k_busy_wait(HANDLE_US)`, `handled++`.

### The catch: "pending" semantics

`k_work_submit()` on an item that's **already queued** does nothing and returns 0. Three edges in the same tick give you **one** handler run. Presses that arrive *while the handler is running* do queue another run, because the item stops being pending when it starts. Your handler must never assume "one run = one press".

### What the tests check

| Test | Expectation |
|---|---|
| one press at 5 | logged at tick 5, and `sysworkq` is running at tick 5 (it's cooperative, priority -1, so `ui` is preempted immediately) |
| 3 presses at 10, plus 1 at 11 | `event_log == "AAAA"` |
| 2×A and 3×B at 20 | `"AABBB"`, and both buttons share the same handler |
| thread context | 5 presses = exactly 10 ms of `sysworkq` CPU (and no work in the ISR) |
| don't block the queue | A at 10 and B at 11: B is handled at tick 12, right after A |

Don't "debounce" with `k_msleep()` in the handler. The system workqueue is shared by everything (Bluetooth, sensors, your code), and a sleeping handler stalls all of it.
