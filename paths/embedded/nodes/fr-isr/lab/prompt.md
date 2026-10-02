# Same-tick barcode bytes

A handheld scanner streams barcode bytes over USART1 into a `decoder` task (priority 4). A `ui` task (priority 1) redraws the display and never sleeps. Field testers complain that the scanner "feels laggy". The timeline shows why: each byte reaches the decoder one tick after its interrupt.

Write `USART1_IRQHandler()`.

**Requirements**
- Read the byte with the provided `uart_read_dr()`.
- Hand the byte to the decoder through `g_rx_queue` with `xQueueSendFromISR()`. You're in an ISR, so `xQueueSend()` is not allowed. The simulator fails the test if you call it, just as a real port would `configASSERT`.
- Use the `xHigherPriorityTaskWoken` pattern: declare it as `pdFALSE`, pass its address, and end the handler with `portYIELD_FROM_ISR(xHigherPriorityTaskWoken)`. The decoder must then run **in the same tick** as the interrupt, preempting `ui`.
- If the queue is full, `xQueueSendFromISR()` returns `errQUEUE_FULL`. Count it in `g_rx_dropped` and return. An ISR never waits.

**What the tests check:** a byte at tick 10 is handled at tick 10. "HELLO" arrives in order and on time. `ui` is preempted right away. Three bytes in one tick are all queued. Ten bytes in one tick overflow the 8-slot queue, and the drops are counted.
