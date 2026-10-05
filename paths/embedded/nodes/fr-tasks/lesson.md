### The RTOS in your toothbrush

Somewhere near you, an electric toothbrush, a smart meter or a car's window lifter is running FreeRTOS right now. It's the most deployed RTOS on the planet. That's not because it's clever: the kernel is a handful of C files, and its API makes you say exactly what you mean. In Fundamentals you built everything on a neutral `rtos.h`. Now you'll port to the real thing, and you'll find you already know 90% of it.

### Same ideas, different spelling

A FreeRTOS task is the task from Fundamentals: a function with its own stack that the scheduler runs, preempts and blocks. Here is how you create one:

```c
BaseType_t xTaskCreate(TaskFunction_t pxTaskCode,   // void fn(void *pvParameters)
                       const char *pcName,           // for debuggers and the timeline
                       uint32_t usStackDepth,        // in WORDS, not bytes
                       void *pvParameters,           // passed to the function
                       UBaseType_t uxPriority,       // 0 .. configMAX_PRIORITIES-1
                       TaskHandle_t *pxCreatedTask); // out: a handle, or NULL
```

It does `rtos_task_create(name, fn, arg, prio)`, with two additions: a stack size and a handle. Three details bite people.

- **Stack depth is in words.** On a 32-bit Cortex-M, `StackType_t` is 4 bytes, so `xTaskCreate(..., 256, ...)` reserves **1 KB**. Port a "1024-byte" budget literally and you've quietly spent 4 KB. A typical review comment reads: *"is this 512 words or did you mean bytes?"*
- **Priorities are 0..configMAX_PRIORITIES-1, and a higher number is more urgent**, just like the neutral kernel. The range is small, though, because each priority level costs RAM for a ready list. Here `configMAX_PRIORITIES` is 8, and priority 0 belongs to the **idle task**.
- **xTaskCreate can fail.** It allocates the stack and the task control block (TCB) from the FreeRTOS heap. If the heap is exhausted it returns `errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY` instead of `pdPASS`. Check it.

### Reading FreeRTOS names

The names look noisy until you see the system. The prefix tells you the type:

| Prefix | Means | Example |
|---|---|---|
| `x` | `BaseType_t` or a handle/struct | `xTaskCreate`, `xQueue` |
| `v` | returns `void` | `vTaskDelay` |
| `ul`, `us`, `uc` | unsigned long / short / char | `ulTaskNotifyTake` |
| `ux` | `UBaseType_t` | `uxTaskPriorityGet` |
| `pv`, `pc` | pointer to void / char | `pvParameters`, `pcTaskGetName` |
| `pd` | "portable define", a macro | `pdTRUE`, `pdPASS`, `pdMS_TO_TICKS` |

After the prefix comes the **file the function lives in** (`Task`, `Queue`, `Timer`, `Semaphore`), so `vTaskDelay` is in tasks.c and returns void. `BaseType_t` is the CPU's natural word. Most "did it work?" functions return it as `pdPASS`/`pdFAIL` or `pdTRUE`/`pdFALSE`.

### Ticks and pdMS_TO_TICKS

FreeRTOS counts time in ticks: `configTICK_RATE_HZ` interrupts per second. Delays take **ticks**, so always convert:

```c
vTaskDelay(pdMS_TO_TICKS(250));   // 250 ms, whatever the tick rate is
```

In this simulator 1 tick is 1 ms, so `pdMS_TO_TICKS(250)` is 250. Ship a bare `vTaskDelay(250)` and someone will change the tick rate to 100 Hz to save power. Your 250 ms blink then becomes 2.5 s. This is why reviewers reject raw tick numbers.

### The idle task, and tasks that end

When `vTaskStartScheduler()` runs, FreeRTOS creates an **idle task** at priority 0. It runs when nothing else can: it frees the memory of deleted tasks, can call your `vApplicationIdleHook()`, and on low-power parts it is where the CPU goes to sleep (tickless idle). Two consequences:

- Give your own tasks priority **1 or above**, unless you really mean "run only when the system is idle".
- Never starve the idle task completely if you delete tasks, because it is the one that frees their memory.

A FreeRTOS task function **must never return**. The neutral kernel let a task fall off the end. FreeRTOS does not: the return address on a fresh stack goes to `prvTaskExitError()`, which asserts and hangs. If a task is done, it deletes itself:

```c
static void boot_task(void *pvParameters) {
    (void)pvParameters;
    run_self_test();
    vTaskDelete(NULL);   // NULL means "me"
}
```

### Worked example: porting three tasks

The Fundamentals version:

```c
rtos_task_create("estop",  estop_task,  NULL, 20);
rtos_task_create("sensor", sensor_task, &cfg, 10);
rtos_task_create("logger", logger_task, NULL, 2);
```

The FreeRTOS version keeps the **order** of the priorities, not their values:

```c
TaskHandle_t sensor;
configASSERT(xTaskCreate(estop_task,  "estop",  128, NULL, 5, NULL)    == pdPASS);
configASSERT(xTaskCreate(sensor_task, "sensor", 256, &cfg, 3, &sensor) == pdPASS);
configASSERT(xTaskCreate(logger_task, "logger", 512, NULL, 1, NULL)    == pdPASS);
```

Inside `sensor_task`, `pvParameters` is `&cfg`. Cast it back with `sensor_cfg_t *cfg = pvParameters;`. That's how one task function can serve several instances, for example "sensor A" and "sensor B" with different configs.

### Gotchas

- **Passing a pointer to a local as `pvParameters`.** If `main()`'s stack frame is reused once the scheduler starts, which it is on many ports, the task reads garbage. Use `static` or global storage.
- **Leaving gaps in priorities "for later"** wastes nothing at runtime. Asking for priority 10 when the maximum is 8 is a configuration error. Real FreeRTOS trips `configASSERT` if you defined it; without asserts it silently clamps the priority to `configMAX_PRIORITIES - 1`, which makes it a silent bug. The simulator fails the test so you notice.
- **Equal priorities time-slice** (`configUSE_TIME_SLICING`). Two busy tasks at the same priority alternate every tick. That's fine for background work, but it's a surprise if you expected one to finish first.

### In the wild

- **Naming tasks** pays off the first time you open a debugger. Segger SystemView, Percepio Tracealyzer and the IDEs' "FreeRTOS task list" views all show `pcName`, so a timeline of `Task1`, `Task2`, `Task3` tells you nothing.
- Common review comments: *"stack depth in bytes or words?"*, *"check the xTaskCreate return value"*, *"why is the logger above the control loop?"*, *"use pdMS_TO_TICKS"*, and *"this task returns, so it will hit prvTaskExitError"*.
- On ESP32, ESP-IDF's `xTaskCreate` takes the stack in **bytes**, which differs from vanilla FreeRTOS. It's a classic porting trap when moving code between an ESP32 and an STM32.
