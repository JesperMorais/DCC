# Port the conveyor controller to FreeRTOS

The conveyor-belt safety controller was prototyped on the neutral `rtos.h` kernel from Fundamentals. The hardware team has picked FreeRTOS for the product, so it's your job to port the design. The comment at the top of the starter shows the original `rtos_task_create` calls, with each task's stack budget in **bytes**.

Write `BaseType_t app_main(void)` and port the four task bodies.

**Creating the tasks**
- Create `"boot"`, `"estop"`, `"sensor"` and `"logger"` with `xTaskCreate()`. Mind the argument order: function, name, stack depth, parameter, priority, handle.
- Stack depth is in **words**, and a `StackType_t` is 4 bytes here.
- Store the handles of estop, sensor and logger in `g_estop_handle`, `g_sensor_handle` and `g_logger_handle`. Boot doesn't need one.
- The neutral design used priorities 30 / 20 / 10 / 2. This FreeRTOS has `configMAX_PRIORITIES` = 8, so valid priorities are 0..7, and **0 belongs to the idle task**. Map the priorities so the order stays the same: boot > estop > sensor > logger > idle.
- Check each `xTaskCreate()` result. Return `pdFAIL` if any of them fails, and `pdPASS` otherwise.

**Porting the bodies**
- Replace `rtos_busy(n)` with `vSimulateWork(n)` and each delay with `vTaskDelay(pdMS_TO_TICKS(ms))`. Keep the same work and delay amounts.
- **boot**: after its 3 ticks of init it must end with `vTaskDelete(NULL)`. A FreeRTOS task function must never return.
- **sensor**: it gets its `sensor_cfg_t` through `pvParameters` (pass `&g_sensor_cfg`). It should count samples into `cfg->samples` and use `cfg->period_ms` as its delay, so tests can reconfigure it.

The tests check the handles and names, the priority order, that boot runs first and deletes itself, that the sensor period comes from its parameter, and that the e-stop poll is never held up by lower-priority work.
