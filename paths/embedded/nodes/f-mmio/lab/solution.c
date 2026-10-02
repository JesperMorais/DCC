#include <stdbool.h>
#include <stdint.h>
#include "mcu.h"

#define LED_PIN 5u     /* PA5 */
#define BUTTON_PIN 13u /* PC13, active-low */

volatile uint32_t *gpio_reg(uintptr_t port_base, uint32_t offset) {
    /* byte offset on the integer address first, then the cast */
    return (volatile uint32_t *)(port_base + offset);
}

void board_init(void) {
    uint32_t moder = GPIOA->MODER;
    moder &= ~(3u << (LED_PIN * 2u));
    moder |= 1u << (LED_PIN * 2u); /* 01 = output */
    GPIOA->MODER = moder;

    GPIOC->MODER &= ~(3u << (BUTTON_PIN * 2u)); /* 00 = input */
}

void led_on(void) {
    GPIOA->BSRR = 1u << LED_PIN; /* set half */
}

void led_off(void) {
    GPIOA->BSRR = 1u << (LED_PIN + 16u); /* reset half */
}

bool button_pressed(void) {
    return (GPIOC->IDR & (1u << BUTTON_PIN)) == 0; /* pressed pulls the pin low */
}
