### The interrupt that ate the motor

Here's a bug that ships in some product every week. A UART interrupt receives a command byte, and somebody thinks "might as well handle it right here". So the ISR parses the command, updates the config, computes a CRC over 2 KB of flash and logs a line. It works on the bench. In the field, a burst of commands arrives while the motor is spinning, the ISR runs for 3 ms, and in that time the motor's encoder interrupts can't run. The position estimate jumps, and the motor faults out. Nobody suspects the "simple" UART handler.

An interrupt handler runs **above every task**. While it runs, the scheduler is frozen. Every millisecond you spend in an ISR is a millisecond of jitter for your highest-priority task.

### Deferred interrupt processing

The fix is to split the work in two:

- **The ISR (top half)** does only what *must* happen at interrupt time: read the data register, clear the flag, and *signal* that something happened. That's microseconds.
- **A handler task (bottom half)** does the real work: parsing, filtering, logging. It's a normal task with a normal priority, so it can be preempted, it shows up in your scheduling analysis, and it's allowed to block.

The signal between them is a **semaphore**. The ISR *gives*, and the task *takes*:

```c
void can_rx_isr(void) {
    rtos_sem_give(rx_sem);              // ISR-safe, never blocks
}

void can_rx_task(void *arg) {
    for (;;) {
        rtos_sem_take(rx_sem, RTOS_WAIT_FOREVER);  // sleep, using zero CPU
        parse_frame();                              // the real work
    }
}
```

The task spends its life asleep inside `take`. When the ISR gives, the kernel makes the task ready. If its priority is higher than whatever was interrupted, it runs **as soon as the ISR returns, on the same tick**. That's why the handler gets a high priority: the work is deferred out of the ISR, not delayed.

```
tick         8   9   10  11  12  13
IRQ                  ^ give
can_rx (3)           ########            runs on the IRQ tick: latency 0
display (2)  ########        ########    preempted, then carries on
```

### Binary vs counting

A semaphore is a counter that can't go below zero. `give` adds one (up to a max), and `take` subtracts one or blocks while it's zero.

- **Binary semaphore** (max 1): "something happened". Two gives before a take still leave just 1.
- **Counting semaphore** (max N): "this many things happened". Each give is remembered.

The difference matters in a **burst**. Say frames arrive at ticks 100, 101, 102, 103, 104, and each takes 2 ticks to parse:

```
tick        100   101   102   103   104   105
frame IRQ    ^     ^     ^     ^     ^
can_rx      [frame 1  ][frame 2  ][frame 3  ][...       2 ticks per frame
binary      hand  =1    LOST  =1    LOST           3 parsed, 2 lost
counting    hand  =1    =2    =2    =3             5 parsed, none lost
```

(The `=n` is the count right after that tick's give. The handler takes one token each time it finishes a frame.)

The first give hands the token straight to the waiting task. With a binary semaphore, the give at 101 sets the count to 1. The gives at 102 and 104 arrive while that 1 hasn't been taken yet, so they're **silently dropped**: `give` just returns `false`. Two of five frames are gone. With a counting semaphore of max ≥ 5, the count climbs and the task works through the backlog, one token per frame. Nothing is lost, as long as the average rate is something you can keep up with.

### The other way to use a binary semaphore

Binary is not "wrong". It's right when the ISR says "the hardware needs attention" and the task **drains** the hardware:

```c
for (;;) {
    rtos_sem_take(rx_sem, RTOS_WAIT_FOREVER);
    while (uart_fifo_not_empty()) handle(uart_read());  // empty it all
}
```

Then a lost give doesn't matter, because one wake-up handles everything that's pending. Choose deliberately: **counting when each give is one event, binary when the task drains a source**.

### Gotchas

- **Never block in an ISR.** `take` with a timeout, `delay` and mutexes are all task-only. There's no task context to put to sleep. Our simulator fails the test with a teaching message, and real kernels assert, or worse, corrupt their lists.
- **Size the count for the worst burst**, not the average. A counting semaphore that saturates loses events just like a binary one. `give` returning `false` is your overflow signal, so count it.
- **A semaphore carries no data.** If the ISR has to pass *what* arrived (a byte, a sample), you need a **queue**. That's the next node.
- **Priority decides latency.** A beautiful deferred handler at priority 1, under a busy display task at 2, never runs. Handlers that the hardware is waiting on go high.
- **Use a timeout as a health check.** `take(sem, 100)` returning false means "no frame for 100 ms". Is the bus dead? A timeout often catches a broken sensor before anything else does.

### In the wild

- **FreeRTOS:** `xSemaphoreGiveFromISR` + `portYIELD_FROM_ISR` (without the yield, the handler waits until the next tick: a classic 1 ms latency bug). Task notifications (`vTaskNotifyGiveFromISR`) are a faster built-in counting semaphore. **Zephyr:** `k_sem_give` from the ISR, or a workqueue item. **Linux:** "top half / bottom half" in exactly these words, with threaded IRQs, tasklets and workqueues.
- **Every network stack** (lwIP, Zephyr's) takes a "packet received" interrupt and defers the protocol work to a thread.
- **Safety standards** (IEC 61508, DO-178C reviews) routinely flag long ISRs. "Keep ISRs short" is a code-review rule in nearly every firmware team.
