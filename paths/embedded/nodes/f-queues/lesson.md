### The sushi belt

Picture a conveyor-belt sushi bar. The chef puts plates on the belt at their pace, and you take them off at yours. Neither of you waits for the other, *until* the belt is full (the chef has to stop, or plates fall off the end) or empty (you sit there hungry). The belt has a fixed number of slots. When the chef is faster than the diners for a while, the belt absorbs the burst.

That belt is an RTOS **queue**. It's the most important building block in firmware architecture, because it lets two tasks with different rhythms work together without sharing a single variable.

### What a queue is

A fixed number of fixed-size slots, with a FIFO order:

```c
rtos_queue_t *q = rtos_queue_create(8, sizeof(uint16_t));  // 8 slots of 2 bytes
rtos_queue_send(q, &sample, timeout);    // copy IN  (blocks/fails if full)
rtos_queue_receive(q, &out, timeout);    // copy OUT (blocks/fails if empty)
```

Three properties make it so useful:

1. **Copy semantics.** `send` *copies* the bytes into the queue. Once it returns, the sender can reuse or destroy its variable, even a local on an ISR's stack. Nothing is shared, so there's nothing to lock.
2. **Blocking.** A receiver on an empty queue sleeps, using zero CPU, until data arrives, just like a semaphore take. A sender on a full queue can wait too. Unlike a semaphore, the wake-up **carries data**.
3. **Built-in synchronisation.** The queue itself is the critical section. Producer and consumer never touch the same memory.

### A pipeline

Real systems chain queues into a pipeline, with each stage at the priority that fits its job:

```
 ADC IRQ ──► [sample_q: 8 x u16] ──► filter (prio 3) ──► [log_q: 4 x i32] ──► logger (prio 1)
 every 5 ms   absorbs CPU stalls      avg of 4, 1 ms      absorbs SD-card     30 ms per line
                                                          hiccups             on a bad day
```

The ISR does the minimum: read the ADC and send with **`RTOS_NO_WAIT`**. The filter wakes on every sample, and every fourth one it produces an average. The logger is slow and low-priority. It writes in the gaps, and `log_q` smooths out its hiccups.

### Back-pressure: what happens when a stage is too slow

When a consumer falls behind, its input queue fills. Then the producer has to choose:

- **Wait** (`send` with a timeout). The slowness propagates upstream, which is *back-pressure*. The filter waits for the logger, so `sample_q` fills, and then the ISR feels it.
- **Drop.** An ISR **can't wait**, so at the head of the pipeline you *must* pick a drop policy:
  - **Drop newest:** the send fails, you count it. You keep the oldest data. That's simple and the default for most streams.
  - **Overwrite oldest:** pop one slot, then push. You keep the freshest data. That's right for "current value" streams like a sensor reading for a display. (FreeRTOS has `xQueueOverwrite` for the 1-slot "mailbox" version.)

```
time      ──────────────────────────────────►
samples   1  2  3  4  5  6  7  8  9  10 11 12 13
queue(8)  [1 2 3 4 5 6 7 8]  full
filter    ...starved by a flash write...      takes 1..8
drop newest:      9 10 11 12 are lost, counter = 4    then 13 14 15 16 → next average
overwrite oldest: 1 2 3 4 are lost, the queue holds 5..12
```

Whichever you choose, **count the drops**. A counter that you can read over telemetry turns a mystery ("the graphs look a bit jagged") into a fact ("we drop 4 samples every time the firmware updater runs").

### Sizing a queue

A queue has to hold everything that arrives while the consumer is **not running**:

```
length  >=  arrival rate  x  longest consumer stall   (+ margin)
```

Samples arrive every 5 ticks. The filter can be starved for 60 ticks by a priority-5 flash write, so that's 12 samples. With 8 slots you lose 4 every time. You can make the queue longer, shorten the stall, or decide that losing them is acceptable and document it. A queue *can't* fix a consumer that's slower **on average** than its producer: it only delays the overflow. Then you need a faster consumer, decimation, or a deliberate drop.

### Worked example: the ISR end

```c
void adc_isr(void) {
    uint16_t sample = adc_read();                        // a local, on the ISR stack
    if (!rtos_queue_send(sample_q, &sample, RTOS_NO_WAIT))
        samples_dropped++;                               // full: drop newest, count it
}                                                        // `sample` dies here, which is fine: it was copied
```

On the consumer end, the filter blocks in `receive` and computes when it has four samples. Then it sends the average to `log_q`, *waiting* if the logger is behind. Task-to-task back-pressure is fine, because a task is allowed to wait.

### Gotchas

- **`item_size` must match what you send.** `rtos_queue_create(8, sizeof(int))` and then sending a `uint16_t*` copies 4 bytes from a 2-byte variable. That's a silent overread. Use `sizeof` of the actual variable type in both places.
- **Queues of pointers** are fast for big items (a 1 KB frame), but then *you* own the lifetime. Never send a pointer to a local. Use a pool of buffers and pass ownership: whoever receives the pointer frees or returns it.
- **Never `WAIT_FOREVER` in an ISR.** Our simulator stops the test, and FreeRTOS has separate `...FromISR` calls that can't block at all.
- **Priorities shape the queue.** If the consumer is *higher* priority than the producer, the queue mostly sits at 0 or 1 items. If it's lower, the queue fills in bursts. Both are fine, as long as you know which one you have.
- **Don't poll.** `while (!receive(q, &x, RTOS_NO_WAIT)) {}` burns the CPU that the producer needs. Block with a timeout instead.

### In the wild

- **FreeRTOS:** `xQueueCreate`, `xQueueSend`/`xQueueReceive`, `xQueueSendFromISR` + `portYIELD_FROM_ISR`, and `xQueueOverwrite`. Stream and message buffers cover byte streams. **Zephyr:** `k_msgq` (copy semantics), with `k_fifo` and `k_pipe` for other shapes. **Linux:** pipes, `mq_send`, `eventfd` + ring buffers.
- **Every telemetry stack** looks like this pipeline: sensors → filtering → packing → radio, with a queue between each stage and a drop counter on each queue.
- **Actor-style firmware** (one queue per task, all communication by messages) is how many teams build large RTOS applications without a single shared mutex. QP/C and "active objects" make it formal.
