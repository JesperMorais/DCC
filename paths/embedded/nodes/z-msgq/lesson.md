### The lag you could feel

Early smartwatch firmware had a familiar complaint: raise your wrist and the screen wakes up half a second later. Often the gesture code was fine. It was just processing **old data**. A burst of radio activity stalled the consumer, the sensor queue filled up, and the code kept the oldest samples and threw away the new ones. The filter then spent the next 50 ms catching up on history.

Every queue eventually fills up. The design question is **what to throw away** when it does, and whether you'll know that you did.

### k_msgq: your Fundamentals queue with a Zephyr name

A message queue holds a fixed number of fixed-size messages, copied in and out. That's exactly the `rtos_queue` from Fundamentals.

```c
struct imu_sample {
    uint32_t seq;
    uint32_t t_ms;
    int16_t  x, y, z;
};

/* name, message size, max messages, alignment of the buffer */
K_MSGQ_DEFINE(imu_q, sizeof(struct imu_sample), 8, 4);
```

- **Messages are copied** in and out, so the producer can reuse its local struct immediately. Keep messages small, a few dozen bytes. For big payloads, queue a pointer or index into a buffer pool instead.
- **The storage is static**: 8 × `sizeof(struct imu_sample)` bytes, reserved at build time. There's no malloc.
- **The alignment argument** must suit your struct (4 is typical on 32-bit MCUs).

The API mirrors `k_sem`, including the return codes:

| Call | Returns |
|---|---|
| `k_msgq_put(&q, &msg, timeout)` | 0, **-ENOMSG** (full and `K_NO_WAIT`), -EAGAIN (timed out) |
| `k_msgq_get(&q, &msg, timeout)` | 0, **-ENOMSG** (empty and `K_NO_WAIT`), -EAGAIN (timed out) |
| `k_msgq_num_used_get(&q)` / `num_free_get` | how full it is |
| `k_msgq_purge(&q)` | empties it |

### ISRs use K_NO_WAIT. Always.

A thread can wait for space with a timeout. An ISR can't. So when an ISR finds the queue full, `k_msgq_put(..., K_NO_WAIT)` returns `-ENOMSG` immediately and **you choose a policy**:

**Drop-newest.** Discard the sample you just made. It's one line, `if (k_msgq_put(...) != 0) dropped++;`, and it's the right choice when *history* matters more than freshness: a log, a command stream, a protocol where the order must be kept.

**Drop-oldest.** Make room by removing the oldest message, then put the new one in:

```c
void imu_isr(void) {
    struct imu_sample s = read_sample();
    if (k_msgq_put(&imu_q, &s, K_NO_WAIT) == -ENOMSG) {
        struct imu_sample stale;
        k_msgq_get(&imu_q, &stale, K_NO_WAIT);   /* throw away the oldest */
        dropped++;
        k_msgq_put(&imu_q, &s, K_NO_WAIT);       /* now there's room */
    }
}
```

That's right for **control and sensing**, where a sample from 20 ms ago is worse than useless. It's safe here because the ISR can't be interrupted by the consumer: the get and the put happen as one unit from the thread's point of view. If a *thread* did the same dance, another producer could slip in between the get and the put, so you'd wrap the pair in `irq_lock()`/`irq_unlock()` (or a lock shared by all producers).

**Whatever you choose, count it.** A `dropped` counter turns "it feels laggy" into a number in your telemetry. The invariant `produced == consumed + dropped + still_queued` is a great debug assertion.

### The consumer

```c
static void fusion_thread(void *a, void *b, void *c) {
    struct imu_sample s;
    for (;;) {
        k_msgq_get(&imu_q, &s, K_FOREVER);       /* sleeps until there's data */
        if (k_uptime_get_32() - s.t_ms > STALE_MS) stale++;
        update_filter(&s);
    }
}
```

Blocking with `K_FOREVER` costs nothing while you wait, because the thread isn't scheduled. Timestamp messages at the source. Then the consumer can *measure* staleness instead of guessing at it.

### Worked example: sizing the queue

A 1 kHz IMU and a BLE thread that's more urgent and occasionally takes 20 ms. In the worst case the consumer is stalled for 20 ms, so 20 samples arrive. Your choices:

- **Depth ≥ 20** (plus margin): nothing is lost, but after the stall you process data that's up to 20 ms old.
- **Depth 8 with drop-oldest**: 12 samples are dropped (and counted), but the consumer resumes with data that's at most 8 ms old.
- **Make the consumer more urgent than BLE**, which in Zephyr means giving it a *smaller* priority number: no stall at all, if the BLE stack's timing can tolerate it. That's a priority decision, not a queue decision.

The queue's depth is a **latency budget** as much as a memory budget.

### Gotchas

- **The kernel can hand a message straight to a waiting thread.** If the consumer is blocked in `k_msgq_get`, a put copies the message directly into its buffer and wakes it. If something more urgent then runs first, the consumer is holding a message that ages. You'll see exactly this in the lab.
- **Don't put pointers to stack variables** into a queue. The message is copied, but what it points to isn't.
- **A full queue in a thread with `K_FOREVER`** is backpressure: the producer waits. That's great between threads, and fatal if the "producer" is your control loop.
- **Message size is fixed per queue.** Variable-length data needs `k_fifo`/`k_pipe`, or a buffer pool such as `net_buf`.

### In the wild

- **Zephyr's sensor subsystem** and many vendor drivers use a message queue (or an RTIO queue) between the interrupt-driven read and the application thread.
- **Wearables on nRF52/nRF53** commonly use small queues with drop-oldest for motion data, plus drop counters that ship in debug telemetry.
- **CAN and Modbus gateways** on STM32 use drop-newest with a counter. There, losing the *latest* frame is better than reordering history, and an overflow counter shows up in the diagnostics register map.
