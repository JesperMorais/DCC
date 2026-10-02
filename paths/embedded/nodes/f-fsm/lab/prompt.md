The lamp has a single button on **PC13** (active-low: pressed reads 0). A **tap** toggles the light and a **hold of one second** switches the lamp off. The button bounces like a cheap tactile switch does.

Write the button logic as an explicit state machine (an `enum` of states plus a `switch`), stepped once per millisecond by `SysTick_Handler`:

```c
#define DEBOUNCE_MS 20u
#define LONG_PRESS_MS 1000u

void     button_init(void);
void     SysTick_Handler(void);       /* 1 kHz: sample PC13, step the FSM */
bool     button_is_pressed(void);     /* the debounced state */
unsigned button_short_presses(void);
unsigned button_long_presses(void);
```

**`button_init`**: configure PC13 as an input (`00` in MODER, other pins untouched), put the FSM in its idle state and zero both counters.

**Debouncing (both edges)**
- On every tick, sample PC13 once.
- A change of the debounced state is accepted only after the raw level has held the *new* value for **`DEBOUNCE_MS` consecutive samples**. A single sample of the old level restarts the wait.
- So glitches and bounces shorter than 20 ms never change anything.

**Events**
- **Long press**: once the button has been (debounced) pressed for **`LONG_PRESS_MS`** ticks, count **one** long press immediately, *while it's still held*. Holding it longer never counts another.
- **Short press**: when a press is released (debounced) *before* it became a long press, count one short press.
- Releasing after a long press counts **nothing**.

The tests drive the "hardware" themselves: `sim_gpio_set_input(GPIOC, 13, level)` moves the pin and `sim_systick(n)` delivers `n` ticks. A test might look like this:

```c
button_init();
press(); sim_systick(2); release(); sim_systick(1); press();  // bounce
sim_systick(200);                                           // stable: held
release(); sim_systick(50);
button_short_presses();   // → 1
```

The tests allow a few ticks either way around the 20 ms and 1000 ms marks, so exact off-by-one choices don't matter. Bounces, glitches and repeat events do.
