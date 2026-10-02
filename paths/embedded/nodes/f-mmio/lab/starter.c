#include <stdbool.h>
#include <stdint.h>
#include "mcu.h"

#define LED_PIN 5u     /* PA5 */
#define BUTTON_PIN 13u /* PC13, active-low */

volatile uint32_t *gpio_reg(uintptr_t port_base, uint32_t offset) {
    (void)port_base;
    (void)offset;
    return &GPIOA->MODER;
}

void board_init(void) {
    /* TODO: PA5 output, PC13 input, nothing else changes */
}

void led_on(void) {
    /* TODO: through BSRR */
}

void led_off(void) {
    /* TODO: through BSRR */
}

bool button_pressed(void) {
    return false;
}
