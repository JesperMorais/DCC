#include <stdbool.h>
#include <stdint.h>

#define PIN_MODE_INPUT 0u
#define PIN_MODE_OUTPUT 1u
#define PIN_MODE_ALT 2u
#define PIN_MODE_ANALOG 3u

uint32_t reg_set_bit(uint32_t reg, unsigned bit) {
    return reg | (1u << bit);
}

uint32_t reg_clear_bit(uint32_t reg, unsigned bit) {
    return reg & ~(1u << bit);
}

uint32_t reg_toggle_bit(uint32_t reg, unsigned bit) {
    return reg ^ (1u << bit);
}

bool reg_test_bit(uint32_t reg, unsigned bit) {
    return (reg & (1u << bit)) != 0;
}

/* A mask of `width` ones. 1u << 32 is undefined, so the full width is a special case. */
static uint32_t field_mask(unsigned width) {
    return width >= 32 ? 0xFFFFFFFFu : (1u << width) - 1u;
}

uint32_t reg_get_field(uint32_t reg, unsigned pos, unsigned width) {
    return (reg >> pos) & field_mask(width);
}

uint32_t reg_set_field(uint32_t reg, unsigned pos, unsigned width, uint32_t value) {
    uint32_t mask = field_mask(width);
    return (reg & ~(mask << pos)) | ((value & mask) << pos);
}

uint32_t moder_set_pin_mode(uint32_t moder, unsigned pin, uint32_t mode) {
    return reg_set_field(moder, pin * 2u, 2u, mode);
}

uint32_t moder_get_pin_mode(uint32_t moder, unsigned pin) {
    return reg_get_field(moder, pin * 2u, 2u);
}
