## Pathfinder on the SPI bus

A rover's flight computer runs three tasks that share an **SPI bus**, protected by the mutex `spi_bus`:

| task | prio | release | work |
|---|---|---|---|
| `attitude` | 3 | every 10 ticks from tick 2 | read the gyro over the bus (1 tick), integrate (1 tick). **Deadline: done within 4 ticks.** |
| `compress` | 2 | every 50 ticks from tick 6 | 10 ticks of image compression. Never touches the bus. |
| `telemetry` | 1 | every 20 ticks from tick 0 | format a packet (3 ticks), send it over the bus (2 ticks) |

In testing, `attitude` misses deadlines, and in flight a watchdog would reset the rover. Fix `telemetry_task` and `rover_start` so that:

1. **`attitude` meets every deadline** over 1000 ticks (100 jobs, each done ≤ 4 ticks after release).
2. **The critical section is short.** Telemetry holds the bus only for the transfer, not while formatting. At tick 2, attitude must get the bus straight away (response 2).
3. **Inversion is bounded.** At tick 12, attitude wants the bus while telemetry is mid-transfer, and `compress` is ready too. Attitude must wait only for the rest of that transfer (response 3), not for the compression. The tests check that `telemetry` was boosted to priority 3 (`sim_max_priority_seen`) and that `compress` never was.
4. **The bus stays protected.** Every `spi_transfer()` happens while its caller owns `spi_bus`. All 50 packets are still sent, and compress still gets all its CPU.

Don't change the attitude or compress tasks, the priorities or the timings. `spi_transfer()` and `attitude_done()` are provided by the board (the tests).

On the Timeline, look for the moment around tick 12. Before the fix, `compress` runs while `attitude` waits for a priority-1 task. After the fix, you'll see telemetry's priority change to 3 and back.
