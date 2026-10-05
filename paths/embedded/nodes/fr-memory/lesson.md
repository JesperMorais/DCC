### The stack that ate the throttle

In 2013, an Oklahoma jury found Toyota liable in a crash blamed on unintended acceleration (*Bookout v. Toyota*). The plaintiffs' embedded expert, Michael Barr, spent months in the engine-control source code. Among his findings, in sworn testimony: the worst-case stack usage could **exceed** the stack, with nothing to detect an overflow. Stack overflow could corrupt the RTOS's own data. And a single task (the infamous "Task X") ran the throttle control *and* several of its failsafes, so one task dying could disable both the function and its safety net. The watchdog didn't catch it, because it only checked that the CPU was alive, not that the critical tasks were running. (NASA's earlier study found no proof of an electronic cause. The jury, hearing Barr, found the software design defective. The RTOS was OSEK-based, not FreeRTOS, but the lessons are universal.)

This lesson is about the boring parts that decide whether a product is safe: **where the memory comes from, and what happens when it runs out.**

### Where FreeRTOS gets memory

Every `xTaskCreate`, `xQueueCreate` and `xTimerCreate` allocates RAM for a stack, a control block or a queue buffer. FreeRTOS gets it from `pvPortMalloc()`, and you choose the implementation by compiling exactly one `heap_N.c`:

| File | Free? | What it is | Use when |
|---|---|---|---|
| `heap_1` | ✗ never | a bump allocator over a static array | everything is created at boot and lives forever. Deterministic and simple, and the most common choice for safety work |
| `heap_2` | ✓ | best-fit, **no coalescing** of free blocks | legacy. Fragments if sizes vary; superseded by heap_4 |
| `heap_3` | ✓ | wraps the C library's `malloc`/`free` with the scheduler suspended | you must share the toolchain's heap. Not deterministic, and size is set by the linker |
| `heap_4` | ✓ | first-fit **with coalescing** of adjacent free blocks | the general-purpose default when you really do create and delete at runtime |
| `heap_5` | ✓ | heap_4 across **several non-contiguous regions** | internal SRAM + CCM + external SDRAM. Call `vPortDefineHeapRegions()` first |

heap_1, heap_2 and heap_4 carve memory out of one array of `configTOTAL_HEAP_SIZE` bytes; heap_5 uses the regions you hand it, and heap_3 uses whatever the linker gave the C library. `xPortGetFreeHeapSize()` tells you how much is left now, and with heap_4/heap_5 `xPortGetMinimumEverFreeHeapSize()` tells you how little was left at the worst moment so far. Log the second one in your test builds.

### Static allocation: no heap at all

Many teams skip the heap entirely. With `configSUPPORT_STATIC_ALLOCATION 1`, every object has a `…Static` twin that takes memory **you** provide:

```c
#define SENSOR_STACK_WORDS 256
static StackType_t  sensor_stack[SENSOR_STACK_WORDS];
static StaticTask_t sensor_tcb;

TaskHandle_t h = xTaskCreateStatic(sensor_task, "sensor", SENSOR_STACK_WORDS,
                                   NULL, 3, sensor_stack, &sensor_tcb);
// never NULL: nothing to run out of

static uint8_t          log_storage[8 * sizeof(log_msg_t)];
static StaticQueue_t    log_qcb;
QueueHandle_t q = xQueueCreateStatic(8, sizeof(log_msg_t), log_storage, &log_qcb);
```

The benefit is that **the linker does your capacity planning**. If it doesn't fit, the build fails, not the product three weeks into a field trial. You also supply memory for the idle task through `vApplicationGetIdleTaskMemory()`, and for the timer task through `vApplicationGetTimerTaskMemory()`. Set `configSUPPORT_DYNAMIC_ALLOCATION 0` and nobody can sneak a `malloc` in. That's the Fundamentals "memory without malloc" lesson, applied to the kernel.

### Sizing stacks: measure, don't guess

A task's stack holds its locals, its call chain, and the **saved context** when it's switched out. On a Cortex-M4F with the FPU in use, that context is about 50 words. ISRs run on the separate main stack (MSP), but the *first* exception frame lands on the task's stack. Big contributors:

- `printf`/`snprintf` with `%f`: 500+ bytes on newlib
- local buffers (`char line[256]`)
- recursion: just don't
- deep driver call chains (HAL → LL → callbacks)

Start generously, run the **worst-case** scenario (every feature on, fault paths taken), then ask FreeRTOS:

```c
UBaseType_t free_words = uxTaskGetStackHighWaterMark(NULL);   // the minimum EVER free, in words
```

FreeRTOS fills new stacks with `0xA5` bytes, and the high-water mark counts how many are still untouched. If it's 20 words after a full test campaign, you're one unusual code path away from disaster. Teams typically aim for 20–30% headroom over the measured worst case.

### Catching overflow and out-of-memory

```c
#define configCHECK_FOR_STACK_OVERFLOW 2
void vApplicationStackOverflowHook(TaskHandle_t t, char *name) {
    // log name to no-init RAM, then reset into a safe state
}

#define configUSE_MALLOC_FAILED_HOOK 1
void vApplicationMallocFailedHook(void) { /* pvPortMalloc returned NULL */ }
```

- **Method 1** checks at each context switch whether the stack pointer is beyond the stack. It's cheap, but it misses an overflow that happened and unwound in between.
- **Method 2** also checks that the last 16 bytes still hold the `0xA5` fill pattern, which catches most transient overflows.

Neither is a guarantee. Both run only at a context switch, and by then the overflow may already have corrupted a neighbour's TCB. The real fix on parts with an MPU is a **guard region** below each stack (the FreeRTOS-MPU ports, or the hardware stack limit registers PSPLIM on Armv8-M), which faults on the *first* bad write.

The **malloc-failed hook** turns a silent `NULL` deep in a driver into a loud, early stop. With heap_1 or static allocation it should only ever fire during development.

### The watchdog lesson from Toyota

A hardware watchdog kicked from a timer interrupt, or from the idle hook, only proves *something* is running. A useful watchdog proves the **things that matter** are running. Each critical task checks in, and a supervisor kicks the hardware watchdog only when **all** of them have checked in recently. You'll build exactly that in the boss.

### Gotchas

- `configTOTAL_HEAP_SIZE` too big for RAM gives a linker error. Too small gives `NULL` at runtime, so check every create.
- **Deleting tasks with heap_1**: their memory is never returned.
- **Stack depth in words** (`xTaskCreate`) vs **bytes** (most other places). Mixing them up is the classic 4× error.
- **Heap fragmentation** over weeks of uptime with heap_4 and varying sizes. Prefer fixed-size pools for runtime allocations.

### In the wild

- MISRA C forbids the standard library's `malloc`/`free` outright (Rule 21.3), and most automotive and medical coding standards **ban dynamic allocation after initialisation**, so heap_1 or full static allocation it is. (SAFERTOS, the certified FreeRTOS derivative, is static-only.)
- Field crash logs from fleets regularly come back as "stack overflow in task X" because someone added a `snprintf("%f")`. That's why high-water marks are often reported in production telemetry.
- Review comments: *"what's the measured high-water mark for this task?"*, *"no configASSERT on xQueueCreate"*, *"enable configCHECK_FOR_STACK_OVERFLOW 2 in debug builds"*, *"the watchdog is kicked from SysTick — that proves nothing"*.
