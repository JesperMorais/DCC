#include <stdbool.h>
#include <stdint.h>
#include "mcu.h"

#define DEBOUNCE_MS 20u
#define LONG_PRESS_MS 1000u

typedef enum {
    BTN_IDLE,
    BTN_PRESS_DEBOUNCE,
    BTN_PRESSED,
    /* TODO: the states you need for long presses and debounced releases */
} btn_state_t;

static btn_state_t state;
static unsigned short_presses, long_presses;

void button_init(void) {
    /* TODO: PC13 as input, FSM idle, counters zeroed */
}

void SysTick_Handler(void) {
    bool raw = (GPIOC->IDR & (1u << 13)) == 0; /* active-low */
    switch (state) {
    case BTN_IDLE:
        if (raw) short_presses++; /* no debounce, no long press... yet */
        break;
    default:
        break;
    }
}

bool button_is_pressed(void) {
    return false;
}

unsigned button_short_presses(void) {
    return short_presses;
}

unsigned button_long_presses(void) {
    return long_presses;
}
