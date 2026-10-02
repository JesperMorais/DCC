# The UART gatekeeper

In the drive controller, the `motor` (priority 5), `sensor` (4) and `ui` (1) tasks all log to one UART. The prototype had every task call `uart_write_line()` directly. That function is slow and **not thread-safe**, so lines came out interleaved and the motor task stalled on serial I/O. Replace the prototype with a **gatekeeper task** that owns the UART.

**Requirements**
- `logger_init()` creates `g_log_q` as a queue of `log_msg_t` **structs**, plus a gatekeeper task named `"uart_gk"` at priority 2. This part is already written.
- `uart_gatekeeper_task` loops forever. It receives a `log_msg_t` (blocking forever is fine here, because the gatekeeper has nothing else to do), formats it as `"<letter>: <text>"` using `source_letter()`, and calls `uart_write_line()`. **Only the gatekeeper** may call `uart_write_line()`. The tests fail any other caller.
- `log_send(source, text)` **copies** `text` into a `log_msg_t`, because callers reuse their buffers immediately. It then queues the message with `xQueueSend()` and a `LOG_SEND_TIMEOUT` (5 ms) timeout. It returns `pdPASS`. If the send times out, it increments `g_log_dropped` and returns `pdFAIL`.
- **Size the queue:** the motor logs bursts of up to **8** lines, and a burst must never block it. A burst of 10 may wait briefly, but it must not lose anything.
- If the UART wedges, a caller loses a few lines but is never blocked for longer than its timeout.

**The tests check that:**
- one line reaches the UART correctly;
- three tasks lose nothing and their lines stay whole and in order;
- 8 lines don't block the motor;
- 10 lines wait without dropping;
- callers can reuse their buffers;
- a 200 ms-per-line UART costs the motor at most about 5 ms per dropped line.
