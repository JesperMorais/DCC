# Keep the pump loop on its grid

A pump's pressure controller runs a 10 ms control loop: read the pressure sensor, run the PID, write the PWM. The PID gains were tuned for **exactly** 10 ms, so a loop that slowly drifts to 12 ms makes the pump oscillate. A low-priority `diagnostics` task meanwhile CRCs the flash and never sleeps.

The starter's `control_task` uses `vTaskDelay(CONTROL_PERIOD)`, so every cycle lasts 10 ms **plus** the 2 ms of work. Fix it.

**Requirements**
- Make the loop drift-free with `xTaskDelayUntil()`. Initialise the wake time with `xTaskGetTickCount()` once, *before* the loop.
- Cycle *k* must start at tick `10·k`, even with diagnostics hogging the CPU. Keep the existing `g_cycle_start[]` logging and `g_cycles` counting.
- **Count overruns.** `xTaskDelayUntil()` returns `pdFALSE` when the deadline had already passed, so the task didn't block. Each time that happens, increment `g_overruns`.
- Don't change the provided code. Tests use `g_slow_cycle` and `g_slow_ticks` to inject a slow cycle.

**What the tests expect.** With one 14-tick cycle starting at 30, there is 1 overrun, the next cycle starts late at 44, and the loop is back on the grid at 50. With a 25-tick stall (30 → 55), both the 40 and 50 deadlines are missed. That gives 2 overruns, and a catch-up cycle runs back-to-back at 55 and 57 before the loop returns to the grid at 60.
