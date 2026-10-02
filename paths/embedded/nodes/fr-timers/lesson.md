### The blink that froze the keypad

A thermostat's firmware had a cute boot animation: in a software-timer callback, the status LED blinked three times with `vTaskDelay(100)` between blinks. It looked great. Then QA noticed that for 600 ms after every boot **the keypad ignored presses**, the backlight timeout fired late, and once the Wi-Fi reconnect timer missed its window entirely. Each of those was a timer too, and they were all stuck behind one callback that went to sleep.

### Software timers: callbacks without a task of your own

A FreeRTOS software timer calls a function of yours **after a delay** (one-shot) or **every period** (auto-reload). You don't need a task per timer: dozens of timers cost a small struct each.

```c
TimerHandle_t xTimerCreate(const char *pcTimerName,
                           TickType_t xTimerPeriod,       // in ticks
                           UBaseType_t uxAutoReload,      // pdTRUE = periodic, pdFALSE = one-shot
                           void *pvTimerID,               // your context pointer
                           TimerCallbackFunction_t pxCallback);

xTimerStart(t, 0);   xTimerStop(t, 0);   xTimerReset(t, 0);   // 0 = don't wait (more below)
```

- **One-shot** (`pdFALSE`): fires once, `period` ticks after it is started or reset. Use it for timeouts: "turn the backlight off 30 s after the last key press".
- **Auto-reload** (`pdTRUE`): fires every `period` ticks until stopped. Use it for heartbeats, polling and blink patterns.
- **`xTimerReset`** restarts the countdown from *now*, and starts the timer if it was dormant. That's the "kick" for an inactivity timeout: every key press pushes the deadline back.

### Where the callback actually runs

In Fundamentals, the neutral kernel ran timer callbacks **in interrupt context**. FreeRTOS doesn't. When `configUSE_TIMERS` is on, it creates a **timer service task** (the "daemon", named `Tmr Svc`) at `configTIMER_TASK_PRIORITY`, which is **6** in this simulator. Your timer API calls and every expiry go through a **timer command queue** to that task, and it calls your callbacks one after another.

```
xTimerStart() ─┐                      ┌─▶ blink_cb()
xTimerReset() ─┼─▶ [timer cmd queue] ─▶ Tmr Svc (prio 6) ─┼─▶ idle_cb()
tick expiry  ──┘                      └─▶ wifi_retry_cb()
```

Two big consequences follow:

1. **Callbacks run in a task, so FreeRTOS calls are allowed.** You can send to a queue or give a notification without the FromISR variants. Inside a callback, though, always use a **block time of 0**.
2. **All callbacks share one task, so a callback must never block.** If `blink_cb` calls `vTaskDelay(100)`, the daemon sleeps for 100 ms, and **every other timer in the system** fires late. Commands from other tasks pile up in the timer queue too. That's the thermostat bug. A callback that waits on a full queue with `portMAX_DELAY` can even stop all software timers forever.

The daemon's priority matters as well. At 6, a timer callback preempts your normal application tasks, so it should be short. It does **not** preempt anything at priority 7. A timer-based watchdog therefore can't run while a priority-7 task spins: keep that in mind for the boss.

### The timer ID: one callback, many timers

`pvTimerID` is a context pointer you get back with `pvTimerGetTimerID()`. It lets one callback serve many timers:

```c
typedef struct { const uint8_t *pattern; uint8_t len, step; bool on; } led_t;
static led_t status_led = { heartbeat, 10, 0, false };
static led_t error_led  = { sos,       18, 0, false };

static void blink_cb(TimerHandle_t t) {
    led_t *led = pvTimerGetTimerID(t);            // which LED is this timer for?
    led->on   = led->pattern[led->step];
    led->step = (led->step + 1) % led->len;
}

xTimerCreate("status", pdMS_TO_TICKS(100), pdTRUE, &status_led, blink_cb);
xTimerCreate("error",  pdMS_TO_TICKS(100), pdTRUE, &error_led,  blink_cb);
```

Here is the non-blocking way to play a pattern: **one step per expiry**, with the state kept in the context struct. It's a small state machine (Fundamentals again) driven by a timer instead of a loop with sleeps.

### Worked example: an inactivity timeout

```c
static void idle_cb(TimerHandle_t t) {
    (void)t;
    backlight_off();
    xTimerStop(blink_timer, 0);     // block time 0: we ARE the daemon
}

void on_key_press(void) {           // called from the keypad task
    backlight_on();
    xTimerReset(idle_timer, 0);     // push the deadline back 1.5 s from now
}
```

Create `idle_timer` one-shot. If you accidentally make it auto-reload, the panel "goes to sleep" again every 1.5 s even while it's already asleep, and your sleep counter keeps climbing.

### Gotchas

- **Calling the timer API with a non-zero block time from inside a callback.** If the timer queue is full, the daemon would wait on its own queue. Use 0.
- **From an ISR, use the FromISR variants:** `xTimerResetFromISR(t, &woken)` plus `portYIELD_FROM_ISR(woken)`.
- **Timer accuracy is the tick.** A 1 ms period at a 1 kHz tick has ±1 tick of uncertainty about when it starts. For µs-level accuracy, use a hardware timer.
- **A "period" of 0 is invalid**, and `configASSERT` fires.
- **The daemon's stack** is `configTIMER_TASK_STACK_DEPTH`. Heavy callbacks with `snprintf` overflow it surprisingly fast.

### In the wild

- BLE stacks (advertising intervals, connection supervision timeouts), Wi-Fi reconnect back-off, key debouncing, auto-power-off and "hold button 3 s for factory reset" are all one-shot or auto-reload software timers.
- AWS IoT and ESP-IDF code bases are full of `xTimerCreate(..., pdFALSE, ...)` timeouts that are reset on activity.
- Review comments: *"vTaskDelay in a timer callback — this stalls every timer"*, *"use the timer ID instead of three copies of the callback"*, *"this should be one-shot"*, *"what's configTIMER_TASK_PRIORITY relative to this task?"*
