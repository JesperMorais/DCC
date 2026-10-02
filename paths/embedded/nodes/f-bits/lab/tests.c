#include <stdbool.h>
#include <stdint.h>

TEST(set_clear_toggle_and_test_low_bits) {
    EXPECT_UEQ(reg_set_bit(0x00u, 3), 0x08u);
    EXPECT_UEQ(reg_set_bit(0x08u, 3), 0x08u);          /* already set: no change */
    EXPECT_UEQ(reg_clear_bit(0xFFu, 0), 0xFEu);
    EXPECT_UEQ(reg_clear_bit(0xF0u, 0), 0xF0u);        /* already clear: no change */
    EXPECT_UEQ(reg_toggle_bit(0x0Fu, 4), 0x1Fu);
    EXPECT_UEQ(reg_toggle_bit(0x1Fu, 4), 0x0Fu);
    EXPECT_TRUE(reg_test_bit(0x20u, 5));
    EXPECT_FALSE(reg_test_bit(0x20u, 4));
}

TEST(bit_31_works_without_signed_shift_ub) {
    EXPECT_UEQ(reg_set_bit(0x00000000u, 31), 0x80000000u);
    EXPECT_UEQ(reg_clear_bit(0xFFFFFFFFu, 31), 0x7FFFFFFFu);
    EXPECT_UEQ(reg_toggle_bit(0x80000001u, 31), 0x00000001u);
    EXPECT_TRUE(reg_test_bit(0x80000000u, 31));
    EXPECT_FALSE(reg_test_bit(0x7FFFFFFFu, 31));
}

TEST(get_field_extracts_and_shifts_down) {
    EXPECT_UEQ(reg_get_field(0xABCD1234u, 8, 8), 0x12u);
    EXPECT_UEQ(reg_get_field(0xABCD1234u, 0, 4), 0x4u);
    EXPECT_UEQ(reg_get_field(0xABCD1234u, 28, 4), 0xAu);  /* the top nibble */
    EXPECT_UEQ(reg_get_field(0xABCD1234u, 31, 1), 0x1u);
}

TEST(set_field_clears_first_and_leaves_neighbours_alone) {
    EXPECT_UEQ(reg_set_field(0xFFFFFFFFu, 4, 4, 0x0u), 0xFFFFFF0Fu);
    EXPECT_UEQ(reg_set_field(0x00000000u, 4, 4, 0xAu), 0x000000A0u);
    EXPECT_UEQ(reg_set_field(0x12345678u, 8, 8, 0xEEu), 0x1234EE78u);  /* 0x56 must be cleared, not ORed */
    EXPECT_UEQ(reg_set_field(0x00000000u, 30, 2, 0x3u), 0xC0000000u);
}

TEST(set_field_masks_values_that_are_too_wide) {
    /* 0x1F doesn't fit in 4 bits: keep 0xF, don't spill into bit 8 */
    EXPECT_UEQ(reg_set_field(0x00000000u, 4, 4, 0x1Fu), 0x000000F0u);
    EXPECT_UEQ(reg_set_field(0x00000000u, 30, 2, 0xFFu), 0xC0000000u);
}

TEST(a_32_bit_wide_field_is_the_whole_register) {
    EXPECT_UEQ(reg_get_field(0xDEADBEEFu, 0, 32), 0xDEADBEEFu);
    EXPECT_UEQ(reg_set_field(0x12345678u, 0, 32, 0xCAFEF00Du), 0xCAFEF00Du);
}

TEST(pin_mode_changes_only_that_pins_two_bits) {
    /* reset value of GPIOA on many STM32s: PA13/PA14 are SWD (alternate) */
    uint32_t moder = 0xABFFFFFFu;
    moder = moder_set_pin_mode(moder, 5, PIN_MODE_OUTPUT);
    EXPECT_UEQ(moder, 0xABFFF7FFu);
    EXPECT_EQ(moder_get_pin_mode(moder, 5), PIN_MODE_OUTPUT);
    EXPECT_EQ(moder_get_pin_mode(moder, 13), PIN_MODE_ALT);
    EXPECT_EQ(moder_get_pin_mode(moder, 14), PIN_MODE_ALT);

    moder = moder_set_pin_mode(moder, 15, PIN_MODE_INPUT);   /* bits 31:30 */
    EXPECT_UEQ(moder, 0x2BFFF7FFu);
    moder = moder_set_pin_mode(moder, 15, PIN_MODE_OUTPUT);
    EXPECT_UEQ(moder, 0x6BFFF7FFu);
    EXPECT_EQ(moder_get_pin_mode(moder, 15), PIN_MODE_OUTPUT);
    EXPECT_EQ(moder_get_pin_mode(0x00000000u, 0), PIN_MODE_INPUT);
}
