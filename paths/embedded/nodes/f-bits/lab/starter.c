#include <stdbool.h>
#include <stdint.h>

#define PIN_MODE_INPUT 0u
#define PIN_MODE_OUTPUT 1u
#define PIN_MODE_ALT 2u
#define PIN_MODE_ANALOG 3u

uint32_t reg_set_bit(uint32_t reg, unsigned bit) {
    (void)bit;
    return reg;
}

uint32_t reg_clear_bit(uint32_t reg, unsigned bit) {
    (void)bit;
    return reg;
}

uint32_t reg_toggle_bit(uint32_t reg, unsigned bit) {
    (void)bit;
    return reg;
}

bool reg_test_bit(uint32_t reg, unsigned bit) {
    (void)reg;
    (void)bit;
    return false;
}

uint32_t reg_get_field(uint32_t reg, unsigned pos, unsigned width) {
    (void)reg;
    (void)pos;
    (void)width;
    return 0;
}

uint32_t reg_set_field(uint32_t reg, unsigned pos, unsigned width, uint32_t value) {
    (void)pos;
    (void)width;
    (void)value;
    return reg;
}

uint32_t moder_set_pin_mode(uint32_t moder, unsigned pin, uint32_t mode) {
    (void)pin;
    (void)mode;
    return moder;
}

uint32_t moder_get_pin_mode(uint32_t moder, unsigned pin) {
    (void)moder;
    (void)pin;
    return PIN_MODE_INPUT;
}
