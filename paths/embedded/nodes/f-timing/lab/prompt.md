## The drifting vibration sampler

A gearbox-monitoring box on a wind turbine samples an accelerometer and feeds an FFT that hunts for bearing wear. The FFT assumes the samples are **exactly 10 ticks apart**. The field team says the frequency peaks are "smeared and slowly sliding". You open the sampler task:

```c
for (;;) {
    uint16_t reading = adc_read();
    rtos_busy(filter_cost(reading));   /* 2 ticks, 6 for a spike, 13 for a sensor fault */
    rtos_delay(SAMPLE_PERIOD);         /* "wait 10 ticks" */
}
```

Fix `sampler_task` so that it is **drift-free**:

- The first sample is taken at the tick the task starts (tick 0 in the tests), then at 10, 20, 30, ... for as long as it runs.
- Varying work (2 or 6 ticks of filtering) must not move later samples.
- **Overrun:** a sensor fault takes 13 ticks, which is longer than a period. The sample after it is taken late, as soon as the work ends (tick 33 in the test). After that the sampler is back on the original grid (40, 50, ...). It does not restart the grid from the late sample.
- **Jitter is OK, drift is not:** if a higher-priority task holds the CPU at a release tick, that one sample is late. The next one is still on the grid.

Keep the task name `"sampler"`, priority 2, and `filter_cost()` as given. `adc_read()` is provided by the board (the tests), and it records when each sample was taken.

Open the **Timeline** tab after a run. With the starter, the sampler's blocks creep to the right. When it's fixed, they line up like fence posts.
