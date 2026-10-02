You've inherited a thermostat's first RTOS port. It has three tasks:

| task | what it does | timing requirement |
|---|---|---|
| `"control"` | reads the sensor and drives the heater: 2 ticks of work | must start **exactly** every 10 ticks (0, 10, 20, …) |
| `"blink"` | toggles the status LED: 1 tick of work | every 50 ticks, a tick or two late is fine |
| `"logger"` | waits until the flash chip says `flash_ready`, then writes a log entry: 3 ticks of work | whenever there's time |

The `flash_ready` flag is set by the flash controller's "done" interrupt. In the tests, that's an ISR raised with `sim_irq_every`.

The previous developer reasoned that "the log is what the customer reads, so it's the most important" and gave the logger the top priority. The logger also waits for the flash like this:

```c
while (!flash_ready) rtos_busy(1);   /* spin until the flash is ready */
```

Since then the room temperature has been swinging wildly. Run the tests and open the **Timeline** tab to see why.

**Fix `app_start` and `logger_task`:**

1. Create the three tasks, named exactly `"control"`, `"blink"` and `"logger"`, with priorities that meet the timing requirements. In `rtos.h`, a **higher number is more urgent**. Use priorities from 1 to 5.
2. Make the logger **block** while it waits for the flash instead of burning CPU. It should check `flash_ready` once per tick and use **no** CPU time while it waits. Its only CPU use is the 3 ticks per log entry.
3. Leave `control_task`, `blink_task` and the work amounts as they are.

The tests check:
- that `control` starts on every multiple of 10
- that `blink` runs within 2 ticks of each 50-tick release
- that every flash-ready event produces a log entry soon afterwards
- that the logger uses CPU only for its real work
- that the CPU spends most of its time **idle**, which on hardware is sleep and battery life
