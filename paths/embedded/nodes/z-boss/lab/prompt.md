## Boss: the sensor hub

You're bringing up the firmware for an industrial vibration hub: an IMU on SPI, a BLE link to the gateway, a UART to the PLC and a flash log. Everything you've learned in this branch goes into one image. The starter compiles but does nothing yet.

```
 imu_isr (every 4 ms) ──k_msgq──▶ processing thread ──k_mutex──▶ stats
                                                                  ▲
 report_timer (100 ms) ──▶ report_work (system workqueue) ────────┤
 logger thread (legacy, given: holds the lock for 5 ms every 50 ms)┘
 ble thread (given): 30 ms of CPU every 100 ms, starting at 51 ms.
```

### Build it

1. **`imu_isr`**: build an `imu_msg` with `seq = ++isr_produced`, `t_ms = k_uptime_get_32()` and `value = imu_value(seq)`. Queue it in `imu_q` **without blocking**. If it doesn't fit, count it in `isr_drops`. (An ISR can't take `stats_lock`, which is why these counters live outside `stats`.)
2. **`processing_thread`**: wait for a message, filter it (`k_busy_wait(FILTER_US)`), then update `stats` **under `stats_lock`**: `processed++`, `sum += value`, and `max_latency_ms` = the worst `now - t_ms`.
3. **Reporter**: a **periodic `k_timer`** (100 ms, first expiry at 100) whose expiry submits `report_work`. Start it in `hub_main`. The handler records `t_ms = k_uptime_get()` at entry, copies `stats` under the lock (a **consistent snapshot**), unlocks, *then* spends `REPORT_US` sending, then appends `{t_ms, processed, sum}` to `reports[]`.
4. **Priorities**: pick `PROCESSING_PRIO`, `BLE_PRIO` and `LOGGER_PRIO`. The system workqueue is fixed at **-1** (cooperative).

Don't change the `logger` and `ble` bodies, the macros or the thread names.

### Acceptance tests

| Test | Pass when |
|---|---|
| no lost messages | 247 messages (4..988 ms): `isr_drops == 0`, all 247 processed, and the sum matches |
| exact reporting | 10 reports in 1050 ms, at **exactly** 100, 200, ..., 1000 |
| thread context | the reports cost 20 ms of `sysworkq` CPU (a mutex or a busy-wait in a timer expiry fails at once) |
| consistent snapshots | every report's `sum` matches its `processed` count, and the counts strictly increase |
| no inversion | at least 245 processed, no drops, and `max_latency_ms <= 10`, even though `logger` holds the lock when BLE bursts begin |
| fairness | `ble` still gets >= 240 ms of CPU per second, and the logger manages >= 15 flash writes |

### Things that will bite you

- **A cooperative processing thread** looks safe, but it delays the system workqueue. Reports land at 101 ms, and "exactly" means exactly.
- **Processing below BLE**: a 30 ms burst is about 7 messages, and the queue holds 4.
- **A semaphore as the lock**: no priority inheritance. `logger` gets stuck behind `ble` while holding the lock, and `processing` waits for both. Watch the Timeline: with a `k_mutex` you'll see `logger` borrow priority.

Use the **Timeline** to check every requirement against what actually ran.
