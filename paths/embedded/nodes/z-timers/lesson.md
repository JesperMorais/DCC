### The timer callback that took down the bus

A very common Zephyr support question goes like this: "My periodic timer reads a sensor over I2C, and it crashes after a few seconds." The code is short and looks reasonable:

```c
static void tick(struct k_timer *t) {
    sensor_sample_fetch(temp);    /* I2C transfer: waits on a semaphore */
}
```

The answer is always the same: **a `k_timer` expiry function runs in interrupt context.** It can't sleep, wait or take a mutex, and an I2C transfer does all three. In Fundamentals your `rtos_timer` callbacks ran in ISR context too. The cure is the same pattern you've just learned: the timer *signals*, and a thread (or the workqueue) *does the work*.

### k_timer: a kernel timer

```c
static void expiry(struct k_timer *t);       /* runs in ISR context */
K_TIMER_DEFINE(sample_timer, expiry, NULL);  /* expiry fn, stop fn (optional) */

k_timer_start(&sample_timer, K_MSEC(100), K_MSEC(100));  /* first, then period */
k_timer_start(&idle_timer,   K_MSEC(500), K_NO_WAIT);    /* one-shot */
k_timer_stop(&sample_timer);                             /* calls the stop fn, if any */
```

- **Duration** is the delay until the first expiry. **Period** is the interval after that, and `K_NO_WAIT` (zero) means **one-shot**.
- **Periodic timers don't drift.** Expiries land on a fixed grid (100, 200, 300...) however long your handling takes, much like `rtos_delay_until` in Fundamentals and unlike `k_msleep(100)` in a loop.
- **Calling `k_timer_start()` on a running timer restarts it** with the new duration. That's how you build "timeout since the last activity".
- **The expiry function may only do ISR-safe things:** give a semaphore, put to a msgq with `K_NO_WAIT`, submit work, write a GPIO, set a flag.

### The pattern: timer → work

```c
static void sample_work_fn(struct k_work *w) {
    sensor_sample_fetch(temp);               /* sleeping on I2C is fine here */
    store(sensor_channel_get(...));
}
K_WORK_DEFINE(sample_work, sample_work_fn);

static void sample_expiry(struct k_timer *t) {
    k_work_submit(&sample_work);             /* ISR-safe, instant */
}
K_TIMER_DEFINE(sample_timer, sample_expiry, NULL);
```

The timer provides drift-free timing. The workqueue provides a thread context that's allowed to sleep. (Zephyr also has `k_work_delayable`, which is a work item with a built-in timer. It's ideal for one-shots and for "do this in 50 ms unless it's rescheduled".)

### Counting the periods you missed

If the system workqueue is stalled (a long handler, or a cooperative thread hogging the CPU), several expiries can happen while `sample_work` is still **pending**. Pending work isn't queued twice, so those expiries collapse into one late run. You'd never know, except that Zephyr counts for you:

```c
uint32_t n = k_timer_status_get(&sample_timer); /* expiries since last call; resets to 0 */
if (n > 1) missed += n - 1;
```

`k_timer_status_get()` returns how many times the timer expired since you last asked, then resets the count. Normally that's 1. Anything above 1 means periods were missed. That's the overrun counter you wrote by hand in Fundamentals, provided by the kernel. (A thread can also call `k_timer_status_sync()` to *block* until the next expiry, which is another clean way to write a periodic thread.)

### Worked example: the backlight timeout

The screen should turn off 500 ms after the last touch:

```c
static void backlight_off(struct k_timer *t) {
    gpio_pin_set_dt(&bl, 0);                 /* a GPIO write: ISR-safe */
}
K_TIMER_DEFINE(bl_timer, backlight_off, NULL);

void touch_isr(...) {
    gpio_pin_set_dt(&bl, 1);
    k_timer_start(&bl_timer, K_MSEC(500), K_NO_WAIT);   /* restart the countdown */
}
```

```
touch:      |300           |700
timer:   0------500x          (restarted, never reaches 500)
                 300------800x  (restarted at 700)
                         700---------1200  OFF
```

Every touch pushes the deadline back, and the light goes off exactly once, 500 ms after the *last* touch.

### Gotchas

- **Remember the flip:** the system workqueue is at -1, more urgent than your app threads *and cooperative*. A cooperative app thread (negative priority) that runs for 200 ms delays your "periodic" samples by up to 200 ms. `k_timer_status_get` will tell you, and the timeline will show you.
- **Don't do real work in an expiry function**, even "quick" work. A `printk` over a slow UART, a flash write or a mutex all belong in a thread.
- **Expiry functions aren't serialised with your threads.** If the expiry writes a variable that a thread reads, make it a single-writer flag or an atomic, or pass it through a queue.
- **Low-power devices:** every timer expiry wakes the CPU. Batch work, use longer periods, and stop timers you don't need (`k_timer_stop`).

### In the wild

- **Thermostats, air-quality monitors, smart meters:** a periodic `k_timer` (or a `k_work_delayable` that reschedules itself) triggers sampling, the I2C/SPI reads happen in work, and the results go to a message queue or the settings subsystem.
- **Inactivity timeouts everywhere:** display backlights, BLE advertising windows, "auto-off after 10 minutes". They're built with a restarted one-shot timer, or with `k_work_reschedule()`, which does the same thing in thread context.
- **Watchdog feeding** is the counterexample. A timer-based feeder proves only that interrupts work. Production code feeds the watchdog from the threads whose health it should prove.
