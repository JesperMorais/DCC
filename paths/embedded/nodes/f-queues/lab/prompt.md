## The soil-moisture pipeline

A solar-powered farm station measures soil moisture. Finish its three-stage pipeline:

```
adc_isr ──► sample_q (8 x uint16_t) ──► filter (prio 3) ──► log_q (4 x int32_t) ──► logger (prio 1)
```

1. **`adc_isr()`** runs every 5 ticks. It reads `adc_read()` and puts the sample into `sample_q`. An ISR must never wait. **If the queue is full, drop the newest sample** (the one you're holding) and increment `samples_dropped`.
2. **`filter`** collects `WINDOW` (4) samples in arrival order, spends 1 tick on calibration (given), and sends their integer average to `log_q`. If `log_q` is full, the filter **waits** (back-pressure).
3. **`logger`** takes averages from `log_q` in order and calls `log_line(avg)` for each one. `log_line` is slow: 3 ticks normally, 22 or 30 on a bad SD-card day.

The tests check that:

- the averages arrive **in order** (sample *n* reads `10·n`, so the first averages are 25, 65, 105, ...),
- at the normal rate **nothing is lost** over 1000 ticks (50 averages, 0 drops),
- when a priority-5 flash write starves the filter for 60 ticks, samples 1–8 are kept, **samples 9–12 are dropped and counted** (4), and the next average is of samples 13–16,
- with a logger slower than the source, the system degrades gracefully: drops are counted, the logger works flat out, the order is preserved and nothing deadlocks,
- the filter (prio 3) preempts a mid-write logger, so it runs on the very tick its window completes.

Keep the queue sizes, priorities and task names (`"filter"`, `"logger"`). `adc_read()` and `log_line()` are provided by the board (the tests).
