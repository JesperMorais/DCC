## The wristband that lagged behind your arm

A fitness wristband samples its accelerometer at **1 kHz**. The "data ready" interrupt (`imu_isr`) captures a sample, and the `fusion` thread (priority 3) turns the samples into gestures. Every now and then the BLE controller's thread (`radio`, priority 1) takes the CPU for **20 ms**. Meanwhile the interrupts keep coming, and the 8-slot queue overflows.

Testers say the gesture detection "lags behind the arm" after every phone sync. That's because the filter is chewing on old data.

### Your job

**`imu_isr`** (interrupt context):
- Build a `struct imu_sample` with `seq = ++produced`, `t_ms = k_uptime_get_32()` and `x = sample_value(seq)`.
- Put it into `imu_q` **without ever blocking** (`K_NO_WAIT`).
- If the queue is full (`-ENOMSG`), apply **drop-oldest**: take the oldest message out (with `K_NO_WAIT` too), count it in `dropped`, then put the new sample in. The newest sample always gets in.

**`fusion_thread`**:
- Block on `imu_q` until a sample arrives.
- Update `stats`: `consumed++`, `sum_x += x`, `last_seq = seq`, and `stale++` if the sample is more than `STALE_MS` (8 ms) old when you consume it.

### What the tests check

| Test | Expectation |
|---|---|
| normal load (50 samples) | every one is consumed, nothing dropped, the sum matches, nothing stale |
| a 20 ms radio stall at tick 10 | `dropped >= 10`, and `consumed + dropped == produced`: nothing vanishes uncounted |
| freshness | at most **1** stale sample per stall |
| two stalls | the ISR never blocks, the counters still add up, and the last sample (#99) gets through |
| the sum | `sum_x` covers exactly the samples that were consumed: #1–#10, #23–#59 |

**Why is one stale sample allowed?** At tick 10, `fusion` is waiting on an empty queue, so the kernel copies sample #10 *straight into its buffer* and makes it ready. But the radio's IRQ fires in the same tick, and the radio is more urgent. Fusion holds #10 in its hand for 20 ms. Look for it on the **Timeline**.
