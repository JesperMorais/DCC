You press the "1" on a cheap microwave and it reads **11**. You press it again and get **111**. The firmware isn't haunted. It's counting every *bounce*.

A mechanical switch doesn't close cleanly. The metal contacts hit, spring apart and hit again, a dozen times in a few milliseconds. Jack Ganssle once put 18 different switches on a scope and saw bounce lasting from under a millisecond to over **150 ms** on one bad switch. To a 100 MHz CPU, that's a dozen presses.

```
raw PC13:  ‾‾‾‾‾‾|_|‾|__|‾|_______________________|‾|_|‾‾‾‾‾‾‾‾‾‾
                 └─bounce─┘   really pressed       └bounce┘
debounced: ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|_____________________________|‾‾‾‾
                      20 ms stable ↑                  20 ms stable ↑
```

Fixing that well teaches the most important structure in small firmware: **the superloop with tick-driven state machines**.

### The superloop

Most small products, such as remote controls, kettles and many sensors, run without any RTOS:

```c
int main(void) {
    init_everything();
    for (;;) {                // the superloop
        button_task();
        display_task();
        radio_task();
    }
}
```

There's one rule that makes it work: **no task may block.** If `button_task()` calls `delay_ms(20)` to debounce, the display freezes and the radio misses its slot for 20 ms. Every job must do a little work and *return*, and remember where it was for next time.

"Remember where it was" is a **state machine**.

### Explicit FSMs: enum + switch + tick

```c
typedef enum { IDLE, PRESS_DEBOUNCE, PRESSED } btn_state_t;
static btn_state_t state = IDLE;
static uint32_t timer;

void button_step(bool raw) {          // called every 1 ms
    switch (state) {
    case IDLE:
        if (raw) { state = PRESS_DEBOUNCE; timer = 0; }
        break;
    case PRESS_DEBOUNCE:
        if (!raw)              state = IDLE;              // just a bounce
        else if (++timer >= 20) { state = PRESSED; on_press(); }
        break;
    case PRESSED:
        /* ... and the same in reverse for release ... */
        break;
    }
}
```

What makes it good:
- **Time is a counter, not a wait.** A 1 ms tick (SysTick, or a timer interrupt) drives `step()`. "Wait 20 ms" becomes "count to 20 while you keep returning".
- **Every state answers every input.** Pressed during debounce? Released during debounce? The `switch` forces you to decide, and those decisions are where the bugs used to hide.
- **It's testable.** Feed it a sequence of `(tick, level)` samples and assert the events. No hardware is needed, and the lab does exactly that.
- **They compose.** Ten buttons are ten small structs stepping in the same loop.

### Worked example: adding a long press

Product says: "a tap toggles the light, and holding for 1 s turns the lamp off". Draw it first:

```
            raw=1                 20 ms stable
  ┌──────┐ ──────▶ ┌──────────┐ ─────────────▶ ┌─────────┐  held 1000 ms  ┌───────────┐
  │ IDLE │         │ DEB_PRESS│                │ PRESSED │ ─────────────▶ │ LONG_HELD │
  └──────┘ ◀────── └──────────┘                └─────────┘   LONG event   └───────────┘
     ▲       raw=0 (bounce)                         │ release (debounced)       │ release
     │                                              ▼ SHORT event               ▼ (no event)
     └──────────────────────────────────── DEB_RELEASE ◀────────────────────────┘
```

Two details are where junior FSMs break:
- The long press must fire **once**, at 1000 ms. That's why `LONG_HELD` is its own state. Otherwise you get "long press, long press, long press…" on every tick after.
- Releasing after a long press must **not** also report a short press. The release path has to remember which state it came from (a second release state, or a flag).

### Where the tick comes from

In a superloop you can call `step()` from the 1 ms **SysTick ISR**, which is precise. Keep the work tiny, because it's an ISR. Or you can let the ISR just set a flag and run `step()` from the loop:

```c
volatile bool tick_flag;
void SysTick_Handler(void) { tick_flag = true; }
...
if (tick_flag) { tick_flag = false; button_step(read_pin()); }
```

The second way keeps ISRs short. The first is fine for a few instructions of FSM. In the lab, the FSM steps inside `SysTick_Handler`.

### Function pointers and dispatch tables

The FSM above calls `on_press()` by name, so the button code is welded to one product. Firmware usually wants "call *whatever* was registered", and C does that with a **function pointer**: a variable that holds the address of a function.

```c
typedef void (*event_fn_t)(void);   // pointer to a function: no arguments, returns nothing

static void toggle_light(void) { ... }
static void lamp_off(void)     { ... }

static const event_fn_t on_event[] = {   // indexed by an enum
    [EV_SHORT] = toggle_light,
    [EV_LONG]  = lamp_off,
};

on_event[ev]();                     // call through the pointer
```

Read the `typedef` inside out: `(*event_fn_t)` is a pointer, and `(void)` after it says it points at a function. Without the parentheses, `void *f(void)` declares a function *returning* `void *`, which is a different thing. A function's name used without `()` is its address, so `toggle_light` goes straight into the array.

That array is a **dispatch table**: data that says what to call, instead of a `switch` that grows a case for every new feature. A table of structs takes it one step further and lets you look things up by name, which is exactly how a command shell works:

```c
typedef int (*cmd_fn_t)(int argc, char *argv[]);
typedef struct { const char *name; cmd_fn_t fn; const char *help; } cmd_t;
```

Walk the table, compare each `name` with what was typed (`strcmp`), and call the match's `fn`. Adding a command is one new row. `help` just walks the same table and prints it. Because the table is `const`, the linker puts it in flash, not RAM.

Two rules keep it safe. **Never call a NULL pointer**: an unset slot is address 0, and on a Cortex-M that's a HardFault (check before calling if slots can be empty). And **the signature must match exactly**: casting a function to a different pointer type and calling it is undefined behaviour, even when it seems to work.

### Gotchas

- **Debounce both edges.** Releases bounce too. A clean press followed by a bouncy release becomes a double-click.
- **Glitches shorter than the window must vanish.** EMI spikes on a long cable look like 2 ms presses. A sample that disagrees *resets* the stability counter.
- **Don't debounce in the ISR with a delay.** A `delay_ms(20)` in an EXTI ISR is the most-copied bad answer on the internet.
- **Overflowing timers.** Use `uint32_t` tick counters and compare with subtraction (`now - start >= 1000`), so a counter that wraps still works.
- **Implicit state.** Five `bool`s (`was_pressed`, `long_sent`, `debouncing`, …) can encode 32 states, and you've thought about maybe 6 of them. An `enum` makes the impossible states unrepresentable.

### In the wild

- **Every keypad, remote and appliance** has a debouncer like this. Many MCUs even have hardware glitch filters on their pins. They help with EMI but rarely cover 20 ms mechanical bounce.
- **Protocol parsers** (NMEA from a GPS, AT commands to a modem, Modbus frames) are byte-driven FSMs in the same enum + switch style.
- **USB, Bluetooth LE and TCP** are specified *as* state machines in their standards documents.
- **Dispatch tables are everywhere:** the Cortex-M vector table is an array of function pointers the CPU indexes on every interrupt, Linux drivers fill a `struct file_operations` with them, and every debug shell (Zephyr's `shell`, U-Boot's commands) is a table of name and handler.
- Tools like **Quantum Leaps' QP**, **SMC** and **Stateflow** generate exactly this code for safety-critical products.

In the lab you'll build the debouncer plus long-press detector as an explicit FSM, stepped by `SysTick_Handler` at 1 kHz. The tests hammer it with bouncy presses, EMI glitches and long holds.
