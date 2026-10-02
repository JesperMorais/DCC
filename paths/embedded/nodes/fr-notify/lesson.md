### The semaphore you didn't need

A fitness band's firmware was fighting for RAM: 64 KB, with BLE eating most of it. Profiling showed 23 binary semaphores, each one about 80 bytes of queue structure, used for a single job: "ISR, wake *that one task*". Replacing them with **task notifications** freed almost 2 KB. The wake-ups also got faster, because a notification doesn't go through the queue machinery at all.

### A semaphore built into every task

Every FreeRTOS task's control block already holds a 32-bit **notification value** and a state. A task notification is a message sent *directly to a task* that updates that value. No separate object, no handle to share, no extra memory.

Used the simplest way, the notification value **is a counting semaphore** owned by the task:

| Fundamentals / semaphores | Task notification |
|---|---|
| `rtos_sem_give(s)` / `xSemaphoreGive(s)` | `xTaskNotifyGive(task)` |
| `xSemaphoreGiveFromISR(s, &woken)` | `vTaskNotifyGiveFromISR(task, &woken)` |
| `xSemaphoreTake(s, timeout)` | `ulTaskNotifyTake(clear, timeout)` |

Give increments the value. Take waits until it's non-zero, and then does one of two things:

```c
uint32_t n = ulTaskNotifyTake(pdTRUE,  timeout); // returns the count, then CLEARS it to 0
uint32_t n = ulTaskNotifyTake(pdFALSE, timeout); // returns the count, then DECREMENTS it by 1
```

Both return the value **before** clearing or decrementing, and both return **0 on timeout**.

- `pdFALSE` behaves like a **counting semaphore**: five gives mean five takes that each return immediately.
- `pdTRUE` behaves like a **binary semaphore** that also tells you how many events piled up. It's handy when you'd handle a burst in one go anyway, for example draining a FIFO until it's empty.

### Worked example: an IMU data-ready interrupt

```c
static TaskHandle_t imu_task_handle;

void EXTI0_IRQHandler(void) {
    BaseType_t woken = pdFALSE;
    vTaskNotifyGiveFromISR(imu_task_handle, &woken);
    portYIELD_FROM_ISR(woken);      // same rule as last lesson
}

static void imu_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        if (ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(50)) == 0) {
            reinit_imu();           // 50 ms of silence: the sensor died
            continue;
        }
        read_one_sample();          // exactly one per interrupt
    }
}
```

There's no `xSemaphoreCreate…` and no handle to share. The ISR only needs the task handle, which you got from `xTaskCreate`'s last argument.

### The pdTRUE trap

Here's a bug that passes every bench test:

```c
for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   // returns 3... and clears to 0
    read_one_sample();                         // ...but we only read one
}
```

On the bench, interrupts arrive one at a time, so the count is always 1. In the field, the task is sometimes preempted by the BLE stack while three samples arrive. `pdTRUE` reports **3**, clears the value, and the code reads one sample. **Two events vanish**, and the step count drifts low. This is exactly the "binary semaphore collapses a burst" bug from Fundamentals, back in new clothes. There are two correct ways to write it:

```c
// counting: one event per take
while (ulTaskNotifyTake(pdFALSE, portMAX_DELAY)) read_one_sample();

// batch: take them all, handle them all
uint32_t n = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
while (n--) read_one_sample();
```

### What notifications can't do

They're faster and lighter, but they're not a drop-in replacement for everything:

- **Exactly one receiver.** The notification belongs to the task. If two worker tasks should compete for events, use a semaphore or a queue.
- **An ISR can't receive one.** Only tasks wait, which is the same rule as everywhere else.
- **No buffering of data.** A notification carries a 32-bit value at most (`xTaskNotify` with `eSetBits`, `eSetValueWithOverwrite`, …). For structs, use a queue.
- **The sender must know the receiver.** That couples them. A queue between two modules lets either side be swapped out.
- **Notification index 0 is shared.** FreeRTOS's own stream buffers and some drivers use a task's default notification. FreeRTOS 10.4 added *indexed* notifications (`ulTaskNotifyTakeIndexed`) so separate subsystems don't step on each other.

### Gotchas

- **Handle not yet valid.** If the interrupt is enabled before `xTaskCreate` has filled in the handle, the ISR notifies `NULL`, which asserts or crashes. Create the task first, then enable the IRQ.
- **Ignoring the 0 return.** A timeout isn't an event. Treating it as one is how a dead sensor becomes a stream of zeros.
- **Blocking on notifications for two different reasons in one task.** A notification doesn't say who sent it. Use bits (`xTaskNotify(t, BIT_RX, eSetBits)` plus `xTaskNotifyWait`) or separate indices.

### In the wild

- FreeRTOS's documentation claims notifications unblock a task up to **45% faster** than a binary semaphore, and they use no extra RAM. That's why driver code (DMA-complete, SPI-done, "radio IRQ") uses them almost everywhere.
- ESP-IDF, Amazon's FreeRTOS libraries and STM32 BSPs all use the "ISR notifies the handler task" pattern.
- Review comments: *"this semaphore has one giver and one taker, so make it a notification"*, *"ulTaskNotifyTake(pdTRUE) but you only process one, so a burst loses events"*, *"handle is NULL if the IRQ fires before task creation"*.
