# Heartbeat LED and backlight timeout

A thermostat's front panel needs two behaviours, both built from **software timers**:

1. **Heartbeat LED.** The status LED plays the pattern in `k_heartbeat`, one entry per 100 ms (`1` = on). That gives a double blink every second. An **auto-reload** timer drives it. Its **timer ID** is `&g_status_led`, and `blink_cb` gets the LED back with `pvTimerGetTimerID()`. On each expiry the callback sets `on` from `pattern[step]` and then advances `step`, wrapping at `len`.
2. **Backlight timeout.** A **one-shot** 1500 ms timer. When it expires, `idle_cb` does the following:
   - sets `g_backlight_on = false`;
   - increments `g_sleep_count`;
   - stores `xTaskGetTickCount()` in `g_sleep_tick`;
   - stops the blink timer;
   - turns the LED off.

**Write:**
- `panel_start()`: create both timers into `g_blink_timer` and `g_idle_timer`, set `g_backlight_on = true`, and start both.
- `user_activity()`, which the keypad task calls on every key press:
  - if the panel is asleep, wake it: backlight on, `step = 0`, and restart the blink timer;
  - in every case, push the timeout back with `xTimerReset()`.

**Rules of the daemon:** both callbacks run in the timer service task "Tmr Svc" at priority 6. They must be short and **must never block**. That means no `vTaskDelay()`, and when you call timer functions from inside a callback, use a block time of 0. If one callback sleeps, every other timer in the system fires late, and the tests will notice.

The tests sample the LED on every tick, check the timer ID, and expect a sleep at exactly 1500 ms, a later sleep after key presses (1500 ms after the last press), exactly one sleep per idle period, and a wake that restarts the pattern from step 0.
