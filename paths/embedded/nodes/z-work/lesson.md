### The interrupt that tried to do everything

A classic first Zephyr driver: the button ISR debounces, writes the LED controller over I2C, logs a message and sends a BLE notification. On the bench it works, mostly. Then I2C *sleeps* while it waits for the bus, which is illegal in an ISR, so the system asserts. Or the BLE stack's timing slips because interrupts are masked for milliseconds. The fix is old wisdom with a Zephyr name: **do the minimum in the ISR, and defer the rest to a thread.** In Fundamentals you did that with a semaphore and a dedicated task. Zephyr gives you a ready-made one: the **workqueue**.

### k_work: "run this function in a thread, soon"

A work item is a small struct holding a handler function. You **submit** it, and the **system workqueue thread** calls the handler as soon as it can.

```c
static void led_work_handler(struct k_work *work) {
    i2c_write_led(...);        /* may sleep on the bus: fine, we're in a thread */
    bt_notify_button();
}
K_WORK_DEFINE(led_work, led_work_handler);

void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    k_work_submit(&led_work);  /* ISR-safe, returns at once */
}
```

That's the whole pattern: no thread to create, no stack to size, no semaphore. For items that live inside structs, use `k_work_init(&obj->work, handler)` at init time.

### The system workqueue is cooperative

The system workqueue is a single thread at priority **-1**, configured with `CONFIG_SYSTEM_WORKQUEUE_PRIORITY`. That's **cooperative**: more urgent than every preemptible thread, and once a handler starts, no other thread can interrupt it. Consequences:

- **Submitted work runs promptly**, ahead of all preemptible application threads (0 and up).
- **Work items run one at a time, in order.** A slow handler delays every other item behind it, and that includes Zephyr's own subsystems: Bluetooth, USB, sensor triggers and the network stack all submit work to the system queue.
- **Never block for long in a handler.** No `k_msleep(20)` "debounce", no waiting for a reply. Use a *delayable* work item (`k_work_schedule`) for delays, or create your own workqueue thread (`k_work_queue_start`) for slow jobs.

### "Pending" means submitting twice is submitting once

A work item is either **idle**, **pending** (queued, not started yet) or **running**. `k_work_submit()`:

| State when you submit | Result |
|---|---|
| idle | queued, returns **1** |
| pending (queued) | **nothing happens**, returns **0** |
| running | queued again, returns **2**: it will run once more after this run |

This is deliberate. Submitting is a *request for attention*, not an event record. But it means:

```
ISR edges:   |  |  |              (3 presses in one tick)
submits:     1  0  0              -> ONE handler run
```

A handler written as "one run = one press" loses presses. The fix is to keep **the event count in your own state**, and have the handler **catch up**:

```c
struct button {
    struct k_work work;
    volatile uint32_t presses;   /* only the ISR writes this */
    uint32_t handled;            /* only the handler writes this */
};

void button_isr(...) { btn.presses++; k_work_submit(&btn.work); }

static void handler(struct k_work *w) {
    struct button *b = CONTAINER_OF(w, struct button, work);
    while (b->handled < b->presses) {
        handle_one_press(b);
        b->handled++;
    }
}
```

A single-writer pair of counters is safe without locks. For flag-style state, Zephyr's `atomic_t` with `atomic_set()`/`atomic_clear()` (which return the old value) is the idiomatic tool.

### CONTAINER_OF: from the work item back to your object

Handlers receive only a `struct k_work *`. To get *your* object, embed the work item in your struct and step back from the member to the struct that contains it:

```
 struct button btn_b
 ┌──────────────────┐ ◀── CONTAINER_OF(w, struct button, work)
 │ struct k_work work│ ◀── w (what the handler receives)
 │ char name         │
 │ uint32_t presses  │
 └──────────────────┘
```

`CONTAINER_OF(ptr, type, field)` subtracts `offsetof(type, field)` from `ptr`. One handler can now serve any number of buttons, sensors or connections, each with its own state, and no globals. The Linux kernel uses the same idiom (`container_of`) everywhere, so you'll read it fluently in both worlds.

### Worked example: a burst

Three presses arrive at tick 10, and one more at 11, while the handler (2 ms per press) is running:

```
tick      10 11 12 13 14 15 16 17 18
ISR       AAA A
sysworkq  #1 #1 #2 #2 #3 #3 #4 #4 (run 2: nothing left, returns at once)
```

Run 1 starts with `presses == 3`. The press at 11 bumps `presses` to 4 and resubmits. The item is *running*, not pending, so it's queued again. Run 1's `while` re-checks the counter after each press, so it handles #4 too. Run 2 then finds `handled == presses` and returns immediately, which is a harmless empty run. With "one press per run", presses #2 and #3 would simply be gone.

### Gotchas

- **Remember the flip:** -1 is *more* urgent than your priority-2 thread, and it's cooperative. A 10 ms handler is a 10 ms stall for every preemptible thread.
- **Don't free or reuse a work item while it's pending or running.** Use `k_work_cancel_sync()` or `k_work_flush()` before tearing an object down.
- **Pending work doesn't carry data.** Data belongs in your struct (or in a `k_msgq` that the handler drains).
- **Each item runs one at a time with itself**, but different items can't interleave either: they share the single workqueue thread.

### In the wild

- **Zephyr's GPIO button samples and input subsystem** defer from the GPIO callback to work, often with a `k_work_delayable` doing the debounce timing.
- **Bluetooth** (`bt_conn` events, GATT notifications) and **USB** both rely on the system workqueue. That's why "my handler took 50 ms and BLE disconnected" is a common nRF forum thread.
- **Sensor trigger drivers** (`CONFIG_<SENSOR>_TRIGGER_GLOBAL_THREAD`) submit work from the data-ready interrupt and read the I2C registers in the handler. It's exactly this pattern, built into the driver.
