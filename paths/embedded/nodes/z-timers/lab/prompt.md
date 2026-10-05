## The thermostat that drifted and never slept

A smart thermostat samples its temperature sensor over **I2C** (3 ms per read, and the driver sleeps while it waits on the bus) and has a touch screen with a backlight. Two complaints from the field:

- The temperature log is "every 100 ms" but the timestamps creep: 101, 205, 309... That's drift. (`k_msleep(100)` sleeps 100 ms *plus one tick*, and it starts *after* 3 ms of work, so the period is 104 ms.)
- The backlight goes dark **while people are using it**, exactly 500 ms after boot. Touches turn it on but never push the timeout back.

### Your job

**1. A periodic sampler, done right: timer → work.**
- A periodic `k_timer` with a 100 ms period (first expiry at 100 ms), started in `app_main`.
- Its expiry runs in **ISR context**, so it only submits a work item. The **work handler** takes the sample: `sample_at[samples++] = k_uptime_get()`, then `k_busy_wait(I2C_READ_US)`.
- **Count missed periods.** A cooperative firmware-update thread (`fwupd`, given) can hog the CPU for 220 ms, which stalls the system workqueue. Expiries that happen while the work is still pending collapse into a single run. Use **`k_timer_status_get()`** in the handler: it returns how many times the timer expired since you last asked. Add anything above 1 to `missed`.
- Remove the old drifting `sampler` thread.

**2. A one-shot timeout, restarted on activity.**
- At boot, `app_main` turns the backlight on and starts a **one-shot** 500 ms `backlight_timer` (period `K_NO_WAIT`). The expiry is given: it switches the backlight off and records when.
- `touch_isr` turns the backlight on **and restarts** the 500 ms timeout. Touches at 300 and 700 mean it goes off at **1200**, once.

### What the tests check

| Test | Expectation |
|---|---|
| grid | samples at exactly 100, 200, ..., 1000 |
| thread context | 30 ms of `sysworkq` CPU for 10 samples (sleeping or I2C in an expiry fails immediately) |
| stall from 250 to 470 | the sample after the stall is at 470, the next one at 500 (the grid holds), `missed == 1`, `samples + missed == 10` |
| idle | the backlight is off at 500, not before |
| touches at 300 and 700 | on at 1199, off at 1200, exactly one off event |
| touch at 600 after a timeout | back on, and off again at 1100 |

