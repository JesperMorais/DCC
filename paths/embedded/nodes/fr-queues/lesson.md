### "MOTOR FAULSENSOR OK 2T 0x3F"

That was a real log line from an industrial drive. Three tasks shared one UART through `printf`. Each one was preempted halfway through its line, and the outputs interleaved. Wrapping `printf` in a mutex fixed the garbling, but then the 1 kHz motor task sometimes waited **8 ms** for the logger's mutex while a slow serial line drained. The design that fixed both problems is the one you'll build: one task **owns** the UART, and everyone else sends it messages.

### The gatekeeper pattern

A **gatekeeper** is a task that is the *only* code allowed to touch a resource. Other tasks don't lock anything. They queue a request, and the gatekeeper performs it.

```
 motor (5) ──┐
 sensor (4) ─┼──▶ [ log queue: log_msg_t × 8 ] ──▶ uart_gk (2) ──▶ UART
 ui (1) ─────┘
```

What you gain:

- **No mutex, no priority inversion, no deadlock.** Nothing is shared, so nothing can be locked in the wrong order. The Fundamentals deadlock lesson simply doesn't apply.
- **Callers don't wait for slow I/O.** The motor task pays for a queue copy, which takes microseconds, not for 20 bytes at 115200 baud.
- **One place for policy.** Timestamps, formatting, rate limiting and "drop debug messages when busy" all live in the gatekeeper.

You'll find gatekeepers everywhere a resource is slow or stateful: UARTs, displays, flash, radios, an I²C bus with several devices.

### Queues of structs, by copy

A FreeRTOS queue **copies** items in and out. You choose the item size when you create it:

```c
typedef struct {
    uint8_t  source;
    uint8_t  level;
    char     text[24];
} log_msg_t;

QueueHandle_t log_q = xQueueCreate(8, sizeof(log_msg_t));   // 8 × 26 bytes, allocated now
```

Copying is the feature, not a cost. The sender can reuse its buffer the moment `xQueueSend` returns, and there's no question of who owns or frees what. This is your Fundamentals queue under FreeRTOS's name. The difference is that the storage comes from the FreeRTOS heap (or from a static buffer with `xQueueCreateStatic`), and the call can fail and return `NULL`.

**Queue pointers only deliberately.** It's fine to queue a pointer for large buffers from a pool, but then the receiver *must* own the buffer until it gives it back. Queuing a pointer to a stack-local `char buf[32]` is the classic bug: by the time the gatekeeper reads it, the sender has overwritten it.

A good habit is a **command and event pair**: requests go into a gatekeeper's *command queue* as a tagged struct (`{ .type = CMD_WRITE, .len = 4, .data = … }`), and results come back on an *event queue*, or through a notification to the requester.

### Sizing a queue

Size it for the **worst burst** the consumer can't drain in time, not for the average:

> The motor task logs up to 8 lines in a fault burst. During that burst the gatekeeper can't run, because the motor has a higher priority. So the queue needs **at least 8** slots, or the motor blocks.

If you can't afford the worst case, decide **what happens when it's full**. That's the timeout.

### Blocking with timeouts

`xQueueSend(q, &msg, timeout)` waits up to `timeout` ticks for space, and `xQueueReceive` waits up to `timeout` for data. The timeout is your policy:

| Timeout | Means | Use for |
|---|---|---|
| `0` | never wait, fail at once | ISRs (via FromISR), hard-real-time senders |
| `pdMS_TO_TICKS(5)` | wait a little, then give up | most producers: ride out a short spike |
| `portMAX_DELAY` | wait forever | a gatekeeper waiting for work |

```c
BaseType_t log_send(uint8_t src, const char *text) {
    log_msg_t m = { .source = src };
    snprintf(m.text, sizeof m.text, "%s", text);           // copy into the message
    if (xQueueSend(log_q, &m, pdMS_TO_TICKS(5)) != pdPASS) {
        dropped++;                                         // a full queue is a decision
        return pdFAIL;
    }
    return pdPASS;
}
```

A short timeout lets a burst that's just over the size wait for the gatekeeper to free a slot, so nothing is lost. If the UART is wedged, the motor loses a few log lines but only ever waits 5 ms, instead of hanging on `portMAX_DELAY` until a watchdog reset. **Logging must never be able to stop the product.**

### Priorities around a gatekeeper

Put the gatekeeper **below** the tasks whose timing matters and above the background. Producers then drop messages and continue, and the gatekeeper drains when they're done. If the gatekeeper outranks the producers, each send preempts the producer, so the motor ends up paying for formatting after all.

### Gotchas

- **Calling the resource "just this once" outside the gatekeeper.** It breaks the whole guarantee. Make the driver `static` to the gatekeeper's file.
- **`uxQueueMessagesWaiting()` to decide whether to send.** That's a race: check-then-act with no lock. Just send with a timeout and look at the result.
- **Variable-length data in fixed slots.** Choose a sensible maximum (`text[24]`) and truncate, or use a **message buffer** (`xMessageBufferCreate`) for variable-size records.
- **Forgetting the receive timeout's return value**, then using a stale struct as a fresh message.

### In the wild

- Zephyr's logging subsystem and ESP-IDF's `esp_log` with a deferred backend are both, at heart, a queue feeding one output task.
- Display drivers (LVGL ports), SD-card loggers and cellular modems (AT-command gatekeepers) use this pattern. So does the FreeRTOS+TCP stack: its IP task is a gatekeeper fed by an event queue.
- Review comments: *"who else calls uart_write?"*, *"queuing a pointer to a stack buffer"*, *"portMAX_DELAY in a producer: what if the consumer dies?"*, *"what's the worst-case burst, and is the queue that deep?"*
