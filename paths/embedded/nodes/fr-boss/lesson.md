### Ship week

The conveyor belt in a parcel-sorting hub moves 3 m/s and carries 40 kg crates. Its motor controller is a Cortex-M4 running FreeRTOS. If the speed loop hiccups, crates slam into the diverters. If the firmware hangs with the motor driven, someone could lose a finger. You have a skeleton, a datasheet and a deadline. Every FreeRTOS tool in this branch fits in one design. Here is the briefing before you start.

### The architecture

```
 USART1 frame ISR ──xQueueSendFromISR──▶ cmd queue ──▶ cmd (5)
                                                        │ writes, under mutex
                                                        ▼
 control (7) ◀── reads setpoint, under mutex ─── g_setpoint ──▶ telemetry (2)
   every 1 ms, xTaskDelayUntil                                   every 10 ms, slow CAN frame
   │ checks in
   ▼
 wdog: 5 ms auto-reload software timer in Tmr Svc (6)
   too few check-ins → trip, motor_disable(), latch PWM to 0
```

Each box is a lesson you've done:

- **control** is `fr-periodic`: drift-free with `xTaskDelayUntil`, overruns counted from its return value. It's the most urgent task in the system, *above* the timer daemon, so no timer callback can ever delay a control cycle.
- **USART1 → cmd** is `fr-isr` and `fr-queues`: a FromISR send of a **struct**, the `xHigherPriorityTaskWoken` pattern, and a dropped-frame counter. A command that arrives at tick *t* is applied at tick *t*.
- **g_setpoint** is the Fundamentals mutex lesson, with FreeRTOS's `xSemaphoreCreateMutex()`, which **has priority inheritance built in**.
- **wdog** is `fr-timers`: a non-blocking callback in the daemon, plus the Toyota lesson from `fr-memory`. The watchdog checks that **the control loop is alive**, not just that the CPU is.

### Three design decisions to get right

**1. Resync after an overrun, don't burst.** For a step counter, catching up missed cycles is right. For a motor loop, catching up means running several cycles back to back on **stale** encoder data and integrating a big error into the PID all at once, which kicks the motor. When `xTaskDelayUntil` returns `pdFALSE`, count the overrun and set `last_wake = xTaskGetTickCount()`. The call didn't block, so one fresh cycle runs right away, and the 1 ms grid restarts from *now* instead of trying to catch up.

**2. Hold locks for a copy, never for work.** Priority inheritance limits inversion, but it doesn't remove the wait. If telemetry holds the setpoint mutex for its 3 ms CAN frame, the 1 ms control task waits up to 3 ms, *with* inheritance, and misses cycles. The FreeRTOS fix is the Fundamentals fix: copy the value under the lock, release, then do the slow part. In the timeline, look for telemetry ever running at control's priority. If it does, it held the lock while control wanted it.

**3. Know what your watchdog can and can't see.** The watchdog callback runs in Tmr Svc at priority 6. If the control task (priority 7) **spins**, the daemon never runs and the software watchdog is blind. That is precisely why real products back it with a **hardware** watchdog (IWDG) that the supervisor kicks only when check-ins are healthy. In this lab the injected fault is a *blocking* hang: an encoder SPI transfer that waits on a DMA completion which comes 20 ms late. Control is blocked, the CPU is free, and the software watchdog can and must trip within about two windows. When it trips, `motor_disable()` turns off the gate driver at once, and the control loop **latches** a PWM of 0, so the motor doesn't restart when the encoder comes back.

A subtle one: the callback **reads and clears** `g_checkins`. Control outranks the daemon, so if a tick wakes control between the read and the clear, it preempts the callback right there, its check-in lands in the gap and is wiped by the clear. (An ISR or a second core on an SMP port can interleave the same way.) Wrap the read and clear in `taskENTER_CRITICAL()`/`taskEXIT_CRITICAL()`. Keep it two lines long.

### How you'll be graded

- Control runs at **every** millisecond for 500 ms while a priority-4 comms stack eats 60% of the CPU, with 0 overruns and no watchdog trip.
- Control's priority is `configMAX_PRIORITIES - 1`, above the timer daemon.
- A command is applied **in the tick** its interrupt fires. Commands apply in order, and speeds are clamped to 0..MAX_RPM.
- **No inversion:** telemetry never runs above its own priority.
- An encoder hang at tick 300 trips the watchdog within ~10 ms, disables the motor and latches PWM to 0. The loop then reports exactly **one** overrun and resumes cleanly at 320, 321, … with no burst.

Use the **Timeline** tab. It's the logic analyser you'd have on the bench: you'll see control's cycles, cmd waking on the USART1 interrupt, telemetry's frames, Tmr Svc's blips and the "encoder SPI hang" marker.

### In the wild

- This is the skeleton of nearly every motor drive, e-bike controller, drone ESC supervisor and industrial servo running an RTOS: a fast loop at the top, commands from a serial or fieldbus, telemetry at the bottom, and a supervisor/watchdog pair that is allowed to pull the plug.
- IEC 61800-5-2 (safe drives) and ISO 13849 call the "turn the gate driver off" path **STO, Safe Torque Off**. In real products it's a hardware path that software can trigger but not block.
- Review comments you've now earned the right to give: *"control below Tmr Svc?"*, *"lock held across the CAN transmit"*, *"no portYIELD_FROM_ISR in the UART handler"*, *"watchdog kicked from a timer that runs even when control is dead"*, *"overrun handling: burst or resync?"*
