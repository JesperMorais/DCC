### One line, one millisecond

A team building a barcode scanner measured their interrupt-to-task latency on a logic analyser. It was 1 ms, every time. Their ISR was 12 instructions long and the decoder task had the highest priority in the system. Yet every byte waited for the *next* SysTick before the decoder ran. The ISR was missing one line: `portYIELD_FROM_ISR()`.

### Two APIs: one for tasks, one for ISRs

In Fundamentals your neutral `rtos_sem_give` was "ISR-safe", because the kernel quietly checked where it was called from. FreeRTOS doesn't do that. It makes you choose explicitly, and gives every API that's allowed in an interrupt a **FromISR** twin:

| In a task | In an ISR |
|---|---|
| `xQueueSend(q, &item, timeout)` | `xQueueSendFromISR(q, &item, &woken)` |
| `xSemaphoreGive(s)` | `xSemaphoreGiveFromISR(s, &woken)` |
| `xTaskNotifyGive(t)` | `vTaskNotifyGiveFromISR(t, &woken)` |
| `xTaskGetTickCount()` | `xTaskGetTickCountFromISR()` |
| `xTimerStart(t, wait)` | `xTimerStartFromISR(t, &woken)` |

The FromISR versions have **no timeout**, because an ISR can never wait. Instead they take a pointer to a `BaseType_t` that they set to `pdTRUE` if the call woke a task with a higher priority than the one that was interrupted.

Why two versions instead of one smart one? Speed and honesty. The task versions may block, may switch context and use a different critical-section mechanism. The ISR versions do none of that, and on Cortex-M they mask only interrupts up to `configMAX_SYSCALL_INTERRUPT_PRIORITY`. Calling `xQueueSend()` from an ISR on real hardware triggers `configASSERT` in a port with asserts enabled, or corrupts the kernel's lists in one without. **The simulator fails the test with a message** (`xQueueSend() called from an ISR — use xQueueSendFromISR()`). Do the same in your real projects: build with `configASSERT` defined.

### The xHigherPriorityTaskWoken pattern

Here is the shape of nearly every FreeRTOS ISR ever written:

```c
void USART1_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;     // 1. start false
    uint8_t byte = USART1->DR;                         // 2. service the hardware
    xQueueSendFromISR(rx_queue, &byte, &xHigherPriorityTaskWoken);   // 3. hand off
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);      // 4. switch on exit if needed
}
```

Step 4 is the one people forget, so here's what it does. The FromISR call moved the decoder task to the ready list, but **the interrupted task is still the one that resumes** when the ISR returns. `portYIELD_FROM_ISR(pdTRUE)` pends a context switch (on Cortex-M it sets PendSV), so that the CPU returns into the **highest-priority ready task** instead. Without it, the decoder only runs at the next scheduling point, which is usually the next tick interrupt.

```
with portYIELD_FROM_ISR:          without:
tick 10   ISR → decoder           tick 10  ISR → ui (resumes!)
          decoder runs                     ui keeps running...
                                  tick 11  SysTick → scheduler → decoder runs
latency: microseconds             latency: up to a whole tick (1 ms here)
```

The simulator models this exactly. A task woken from an ISR without the yield becomes runnable at the **next** tick. You can see the one-tick gap in the Timeline tab.

### Worked example: several events, one yield

The variable is an accumulator. Pass the same address to every FromISR call in the handler and yield **once** at the end:

```c
void DMA1_Stream0_IRQHandler(void) {
    BaseType_t woken = pdFALSE;
    if (half_transfer())  xSemaphoreGiveFromISR(half_sem, &woken);
    if (full_transfer())  xSemaphoreGiveFromISR(full_sem, &woken);
    portYIELD_FROM_ISR(woken);
}
```

If the queue is full, `xQueueSendFromISR` returns `errQUEUE_FULL`. The ISR can't wait for space, so it **counts the drop and moves on**. A dropped-bytes counter you can read over the debug console is worth its weight in gold during field testing.

### Gotchas

- **Not initialising `woken` to `pdFALSE`.** Stack garbage then forces a pointless context switch on almost every interrupt.
- **Interrupt priorities on Cortex-M.** An ISR that calls *any* FreeRTOS API must have a priority numerically **≥** `configMAX_SYSCALL_INTERRUPT_PRIORITY`, which means logically lower. The STM32 HAL's default priority of 0 is the highest, and it's *not allowed*. It's the number one cause of "random" hard faults in FreeRTOS projects, and `configASSERT` inside `vPortValidateInterruptPriority()` catches it.
- **Doing the work in the ISR.** Parse, decode and log in the task. The ISR reads the hardware and hands off. That's the deferred-interrupt pattern from Fundamentals, unchanged.
- **The simulator's ISR is time-free.** On real silicon, an ISR's run time comes straight out of whatever task it interrupted. Keep it short.

### In the wild

- Every FreeRTOS driver layer, including STM32Cube's FreeRTOS integration, NXP's MCUXpresso SDK, Nordic's nRF5 SDK and ESP-IDF's UART driver, follows this exact four-step shape. Learn to spot it and you can read all of them.
- Review comments: *"missing portYIELD_FROM_ISR — this adds up to a tick of latency"*, *"xSemaphoreGive in an ISR"*, *"what's the NVIC priority of this IRQ?"*, *"count the queue-full case"*.
- `portEND_SWITCHING_ISR(x)` is the same macro under an older name, and you'll still find it in ports and legacy code.
