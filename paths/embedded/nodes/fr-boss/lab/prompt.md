# Boss: the conveyor motor controller

It's ship week. The conveyor's motor controller board has four jobs, and the skeleton compiles but does almost nothing. Finish it. Every FreeRTOS idea in this branch is in here.

```
 USART1 frame ISR ──xQueueSendFromISR──▶ g_cmd_q ──▶ cmd (5) ──┐
                                                               ▼  g_setpoint_mutex
 control (7) every 1 ms ◀── reads setpoint ────────────── g_setpoint ──▶ telemetry (2) every 10 ms
      │ g_checkins++                                                      (3 ms CAN frame)
      ▼
 "wdog" timer, 5 ms, in Tmr Svc (6): too few check-ins → trip, motor_disable()
```

**1. Control loop** (`control_task`)
- It must be the **most urgent task in the system**, above the timer daemon. Set `PRIO_CONTROL` to that.
- It runs every 1 ms (`CONTROL_PERIOD`) and must be drift-free.
- Each cycle:
  - log the tick into `g_ctrl_tick[g_ctrl_cycles]` and increment `g_ctrl_cycles`;
  - increment `g_checkins`;
  - call `encoder_read()`;
  - read `g_setpoint` **under `g_setpoint_mutex`**;
  - write `g_pwm` (the setpoint, or `0` once `g_wd_tripped` is set, so the fault latches).
- If the deadline has already passed, `xTaskDelayUntil()` returns `pdFALSE`. Count that in `g_ctrl_overruns`, then **resync** the wake time to now. A motor loop must not fire a burst of stale catch-up cycles.

**2. Commands**
- `uart_cmd_isr(cmd)` runs in interrupt context. It queues the command to `cmd_task`. If the queue is full, it increments `g_cmds_dropped`. The command must be applied **in the interrupt's tick**.
- `cmd_task` handles each command:
  - `CMD_SET_SPEED` is clamped to `0..MAX_RPM`;
  - `CMD_STOP` sets 0;
  - it writes `g_setpoint` under the mutex, records `g_cmd_applied_tick`, and increments `g_cmds_applied`.

**3. Telemetry.** A colleague wrote `telemetry_task`, and code review flagged it. A 3 ms frame and a 1 ms control loop are sharing one lock. Fix it.

**4. Watchdog**
- Create an auto-reload software timer, `WDOG_PERIOD` (5 ms), with `wdog_cb` as its callback, and start it.
- Each expiry, the callback reads and clears `g_checkins` **atomically**. If fewer than `WDOG_MIN_CHECKINS` arrived, it trips **once**: it sets `g_wd_tripped`, records `g_wd_trip_tick` and calls `motor_disable()`.
- It runs in the timer daemon, so it never blocks.

**Graded on:**
- 1 ms timing under a CPU-hungry comms task;
- control's priority;
- same-tick command application;
- ordering and clamping of commands;
- **no priority inversion** (telemetry must never inherit control's priority);
- the watchdog catching an injected 20 ms encoder hang at tick 300 within about 2 windows, with the motor made safe;
- the loop resyncing after the stall: exactly 1 overrun, cycles at 300, 320, 321, …
