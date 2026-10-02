#include <stdbool.h>
#include <stdint.h>
#include "mcu.h"

#define DEBOUNCE_MS 20u
#define LONG_PRESS_MS 1000u

typedef enum {
    BTN_IDLE,                 /* released, stable */
    BTN_PRESS_DEBOUNCE,       /* went low: is it real? */
    BTN_PRESSED,              /* held, not long yet */
    BTN_LONG_HELD,            /* long press already reported */
    BTN_RELEASE_DEBOUNCE,     /* went high after PRESSED: a short press if it sticks */
    BTN_RELEASE_DEBOUNCE_LONG /* went high after LONG_HELD: no event if it sticks */
} btn_state_t;

static btn_state_t state;
static uint32_t stable_ticks; /* consecutive samples at the new level (debounce states) */
static uint32_t held_ticks;   /* how long the debounced press has lasted */
static unsigned short_presses, long_presses;

static void go(btn_state_t next) {
    state = next;
    stable_ticks = 0;
}

void button_init(void) {
    GPIOC->MODER &= ~(3u << (13u * 2u)); /* PC13 input */
    state = BTN_IDLE;
    stable_ticks = 0;
    held_ticks = 0;
    short_presses = 0;
    long_presses = 0;
}

void SysTick_Handler(void) {
    bool raw = (GPIOC->IDR & (1u << 13)) == 0; /* active-low */

    switch (state) {
    case BTN_IDLE:
        if (raw) go(BTN_PRESS_DEBOUNCE);
        break;

    case BTN_PRESS_DEBOUNCE:
        if (!raw) {
            go(BTN_IDLE); /* a bounce or a glitch */
        } else if (++stable_ticks >= DEBOUNCE_MS) {
            held_ticks = 0;
            go(BTN_PRESSED);
        }
        break;

    case BTN_PRESSED:
        if (!raw) {
            go(BTN_RELEASE_DEBOUNCE);
        } else if (++held_ticks >= LONG_PRESS_MS) {
            long_presses++; /* exactly once: LONG_HELD never counts again */
            go(BTN_LONG_HELD);
        }
        break;

    case BTN_LONG_HELD:
        if (!raw) go(BTN_RELEASE_DEBOUNCE_LONG);
        break;

    case BTN_RELEASE_DEBOUNCE:
        if (raw) {
            go(BTN_PRESSED); /* bounce: still held */
        } else if (++stable_ticks >= DEBOUNCE_MS) {
            short_presses++;
            go(BTN_IDLE);
        }
        break;

    case BTN_RELEASE_DEBOUNCE_LONG:
        if (raw) {
            go(BTN_LONG_HELD);
        } else if (++stable_ticks >= DEBOUNCE_MS) {
            go(BTN_IDLE); /* the long press was already reported */
        }
        break;
    }
}

bool button_is_pressed(void) {
    return state == BTN_PRESSED || state == BTN_LONG_HELD || state == BTN_RELEASE_DEBOUNCE ||
           state == BTN_RELEASE_DEBOUNCE_LONG;
}

unsigned button_short_presses(void) {
    return short_presses;
}

unsigned button_long_presses(void) {
    return long_presses;
}
