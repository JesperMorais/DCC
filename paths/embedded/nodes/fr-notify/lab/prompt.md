# Notify the step counter

A wearable's accelerometer raises **EXTI0** for every sample it pushes into its FIFO. The `imu` task (priority 4) reads one sample per event with the provided `read_one_fifo_sample()`. The starter wakes it with a binary semaphore, and code review found two bugs:

1. A burst of FIFO interrupts in one tick gives a single wake-up, so samples are lost and steps go uncounted.
2. The power manager, which isn't your code, signals motion-wake with `xTaskNotifyGive(g_imu_task)`. The imu task never sees it.

Replace the semaphore with a **direct-to-task notification**.

**Requirements**
- `EXTI0_IRQHandler()` uses `vTaskNotifyGiveFromISR(g_imu_task, &xHigherPriorityTaskWoken)` and then `portYIELD_FROM_ISR()`, so the read happens in the same tick.
- `imu_task` waits with `ulTaskNotifyTake(...)` and a timeout of `IMU_SILENCE_TIMEOUT` (50 ms). Every event, from the ISR or from another task, must give exactly one `read_one_fifo_sample()`. Events must not be lost when several arrive at once or arrive during a read.
- If the take times out (it returns **0**), increment `g_timeouts` and wait again. The timeout restarts each time you call the take.
- Keep `g_imu_task` as the imu task's handle, because the ISR and the power manager need it. Remove the semaphore.

You can either decrement one event at a time (`pdFALSE`) or take them all and loop (`pdTRUE`). Both work if you do it right. Taking all of them and then reading only one sample does not work.
