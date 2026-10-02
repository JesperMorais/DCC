### The pump that hummed

A water-pump controller passed every bench test. In the field it hummed, and the bearings wore out early. The PID was tuned for a 10 ms loop, but the loop ran every 10 ms *plus however long the PID took*: 11.3 ms, then 11.8 ms after a feature added another filter. The gains no longer matched the plant, and the motor oscillated slightly, all day long. The fix was one line.

### vTaskDelay is relative, vTaskDelayUntil is absolute

You met this in Fundamentals as `rtos_delay` and `rtos_delay_until`. FreeRTOS has the same pair:

```c
vTaskDelay(pdMS_TO_TICKS(10));                // "sleep 10 ms from NOW"
xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10)); // "wake at last_wake + 10, then advance last_wake"
```

With `vTaskDelay`, "now" is whenever the work finished, so every cycle's work time and every preemption is added to the period. The error **accumulates**, which is called drift:

```
vTaskDelay(10), 2 ms of work:
  |work|....10....|work|....10....|work|
  0    2          12   14         24   26     → period 12 ms, drifting

xTaskDelayUntil(&last, 10):
  |work|..8..|work|..8..|work|
  0    2     10   12    20         → period 10 ms, locked to the grid
```

`xTaskDelayUntil` remembers the *intended* wake time in `last_wake` and adds the period to it, so the loop stays on a grid that never moves. Initialise `last_wake` **once**, before the loop:

```c
static void control_task(void *pvParameters) {
    (void)pvParameters;
    TickType_t last_wake = xTaskGetTickCount();   // once, not in the loop!
    for (;;) {
        run_pid();
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));
    }
}
```

The older `vTaskDelayUntil` does the same thing but returns nothing. `xTaskDelayUntil` (FreeRTOS 10.4.3 and later) returns **whether it actually blocked**, which brings us to the second job of a periodic loop.

### Detecting overruns

If one cycle takes longer than the period, the next wake time is already in the past. `xTaskDelayUntil` then returns immediately with **`pdFALSE`**. That return value is your overrun detector:

```c
if (xTaskDelayUntil(&last_wake, PERIOD) == pdFALSE) {
    overruns++;            // log it, count it, trip a fault if it keeps happening
}
```

Here's the timeline for a single 14 ms cycle starting at 30 in a 10 ms loop:

```
30 ──── slow cycle ───── 44 | 44─46 | 50─52 | 60─62
          deadline 40 missed  ↑ late    ↑ back on the grid
                       returns pdFALSE
```

`last_wake` was advanced to 40, which is already past, so the next cycle starts at once at 44. Its deadline of 50 is in the future, so the loop is back on the grid. A **25 ms** stall misses *two* deadlines (40 and 50). FreeRTOS then runs a quick catch-up cycle immediately (at 55, then 57) before blocking again until 60. Whether catching up is what you want depends on the job. A step counter wants every cycle. A motor loop working from stale sensor data probably wants to **resync** instead (`last_wake = xTaskGetTickCount();`). You'll make that choice in the boss.

### Jitter: why priority matters too

Being drift-free doesn't make a loop jitter-free. `xTaskDelayUntil` makes the task *ready* at exactly 10, 20, 30… It *runs* then only if nothing more urgent is running. A higher-priority task, or a long critical section, shifts each start by a varying amount. That variation is **jitter**. The playground below shows it: `control` is released every 10 ticks, but `radio` sometimes gets there first.

```playground
{
  "title": "Drift-free, but with jitter",
  "ticks": 60,
  "editable": true,
  "tasks": [
    { "name": "radio", "priority": 3, "period": 15, "wcet": 3 },
    { "name": "control", "priority": 2, "period": 10, "wcet": 2 },
    { "name": "diagnostics", "priority": 1, "period": 60, "wcet": 30 }
  ]
}
```

Try swapping the priorities of `radio` and `control`. Control's start times snap to exactly 0, 10, 20…, and radio gets the jitter instead. The rule: **the loop whose timing matters most gets the highest priority**. A low-priority hog like `diagnostics` can't hurt either of them. This is the preemption you studied in Fundamentals, doing its job.

### Gotchas

- **Re-reading the tick count inside the loop** (`last_wake = xTaskGetTickCount();` before each delay) turns `xTaskDelayUntil` back into `vTaskDelay`. It's the most common way to "use DelayUntil" and still drift.
- **Ignoring the return value.** An overrun you don't count is one you won't see until the field returns start.
- **Periods that aren't a whole number of ticks.** A 1 kHz tick can't give a 2.5 ms loop. Use a hardware timer interrupt for that, or change the design.
- **Tick-count wrap-around:** `TickType_t` wraps after 49.7 days at 1 kHz with 32-bit ticks. `xTaskDelayUntil` handles the wrap correctly. Hand-rolled `if (now >= deadline)` comparisons don't. Compare with `(TickType_t)(now - start) >= period` instead.

### In the wild

- Motor drives, power converters, audio and IMU fusion all run fixed-rate loops. Their tuning (PID gains, filter coefficients) assumes **one exact sample period**.
- Review comment: *"vTaskDelay in a control loop: this drifts by the execution time, use xTaskDelayUntil."* A close second: *"what happens when this overruns? Count it."*
- Safety-related firmware (IEC 61508, ISO 26262) often requires **deadline monitoring**. An overrun counter fed from `xTaskDelayUntil`'s return value, checked by a watchdog, is a cheap way to provide it.
