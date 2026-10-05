## Boss: the vaccine-fridge monitor

A clinic's vaccine fridge has to stay between **2.0 and 8.0 °C**. If it drifts out, or if nobody notices the sensor has died, a whole shelf of doses goes in the bin. You're finishing the monitor's firmware. Every Fundamentals idea is in here, and the starter compiles but does almost nothing.

```
 sensor ──bytes──▶ USART1_IRQHandler ──g_byte_q (8)──▶ parser ──data lock──▶ g_stats
 (a 4-byte frame                                       (FSM)                   ▲  ▲  ▲
  every 25 ms)                     alarm, every 100 ms: PB5 buzzer ◀─────────┘  │  │
                                   uplink, every 1 s: report over the modem ◀───┘  │
                  given: display (prio 2) 70 ms redraw every 200 ms               │
                  given: eeprom  (prio 1) holds the lock 3 ms every 500 ms ───────┘
```

### Build it

1. **`hw_init`**: in `USART1->CR1`, set `UE`, `RE` and `RXNEIE`. Make **PB5** an output (`01` in `GPIOB->MODER`) and drive it low. **Every other bit stays as it was.**
2. **`USART1_IRQHandler`**: if `RXNE` is set, read the byte from `DR` and acknowledge it (clear `RXNE` and `ORE` in `SR`). Then queue it in `g_byte_q`. An ISR **never waits**, so if the queue is full, count the byte in `g_bytes_dropped`.
3. **`parser_task`**: a `parse_state_t` state machine over the bytes. A frame is `0xA5`, `MSB`, `LSB`, `CHK`.
   - Until it sees `0xA5`, the parser ignores bytes.
   - If `CHK != MSB ^ LSB`, it counts `g_bad_frames` and goes back to waiting for `0xA5`.
   - Otherwise the reading is the signed 16-bit `(MSB << 8) | LSB`, in tenths of a degree. Pass it through `calibrate()` (1 ms), then store it in `g_stats` **under the data lock**: `latest`, `latest_tick = rtos_now()`, `frames++`, and update `min` and `max`.
   - A data byte can equal `0xA5`. It's still data.
4. **`alarm_task`**: every 100 ms, **on the dot**, starting at tick 0:
   - call `alarm_self_check()` (2 ms);
   - copy what you need from `g_stats` under the lock;
   - drive PB5 **high** if the last valid frame is more than `SENSOR_TIMEOUT` (300 ms) old (counting from boot if none has arrived yet), or if there is a reading and it's outside `TEMP_LOW..TEMP_HIGH`. Otherwise drive it **low**. Don't touch the other GPIOB pins.
5. **`uplink_task`**: every second (the first report goes out at about tick 1000), take a `report_t` of `frames`, `min` and `max`, and **reset `min`/`max`** for the next window, all under the lock. Then hand it to `modem_send()`, which takes 40 ms.
6. **Priorities**: pick `PRIO_ALARM`, `PRIO_PARSER` and `PRIO_UPLINK`. The display (2) and the EEPROM writer (1) are fixed.
7. **The data lock**: a colleague wrote `data_lock()`/`data_unlock()` in a hurry. Review them.

Don't change the provided code, the macros or the task names.

### Acceptance tests

| Test | Pass when |
|---|---|
| hardware setup | `CR1` gains `UE`, `RE` and `RXNEIE` and keeps `TE`. PB5 becomes `01` and goes low. PB0 is untouched in `MODER` and `ODR` |
| decode latency | the frame that arrives at 60, in the middle of a redraw, is stored at **61**. After 1000 ms: 40 frames, nothing dropped, nothing bad |
| bad input | line noise is skipped, a bad checksum counts 1, -4.0 °C decodes as `-40`, and the frame `A5 00 A5 A5` decodes as `0x00A5` |
| burst | 12 bytes in one interrupt: 8 queued, 4 dropped and counted, `RXNE` cleared, and the parser is still in step for the next frame |
| on the dot | alarm checks start at **exactly** 0, 100, ..., 900, even when a frame arrives on the same tick (at 300 and 700), and use 20 ms of CPU in total |
| excursions | 9.5 °C turns PB5 on at the check at 600, 5.0 °C turns it off at 700, -1.5 °C turns it on at 800. PB0 stays on throughout |
| silent sensor | after the last frame at 485, PB5 is still off at 750 and on at 850 |
| no inversion | the frame at 235 is stored by 239, even though `eeprom` holds the lock and the display starts a 70 ms redraw. `eeprom` is seen above priority 2; `display` never is |
| uplink | reports at 1000–1005 and 2000–2005 with frames 40/80, and min/max 31/70 then 30/69. The frames at 1010 and 2035 are stored at 1011 and 2036, while the modem is transmitting. `uplink` is never seen at priority 2 or above, and the display redraw that's due at 2035 runs at 2036, straight after the parser |
| soak | 3 s: no deadlock, 120 frames, nothing dropped, 2 reports |

### Things that will bite you

- **The parser below the display**: a 70 ms redraw spans three frames, which is 12 bytes, and the queue holds 8.
- **The parser above the alarm**: the 1 ms calibration lands on the alarm's tick, and "on the dot" means exactly that.
- **`rtos_delay(100)`** after 2 ms of self-check makes the period 102. By the tenth check you're 18 ms late.
- **Sending while holding the lock**: 40 ms of modem time blocks the parser.

Use the **Timeline**: the redraws, the EEPROM page writes and the modem are all marked.
