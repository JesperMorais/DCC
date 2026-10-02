#include <stdbool.h>
#include <stdint.h>
#include "mcu.h"

TEST(gpio_reg_turns_base_plus_byte_offset_into_the_register) {
    EXPECT_PTR_EQ(gpio_reg((uintptr_t)GPIOA, 0x00), &GPIOA->MODER);
    EXPECT_PTR_EQ(gpio_reg((uintptr_t)GPIOA, 0x04), &GPIOA->IDR);
    EXPECT_PTR_EQ(gpio_reg((uintptr_t)GPIOA, 0x08), &GPIOA->ODR);
    EXPECT_PTR_EQ(gpio_reg((uintptr_t)GPIOC, 0x0C), &GPIOC->BSRR);
    /* and a write through it really reaches the peripheral */
    *gpio_reg((uintptr_t)GPIOB, 0x08) = 0x1234u;
    EXPECT_UEQ(GPIOB->ODR, 0x1234u);
}

TEST(board_init_sets_only_the_two_mode_fields) {
    GPIOA->MODER = 0xABFFFFFFu; /* reset value: PA13/PA14 = SWD */
    GPIOC->MODER = 0xFFFFFFFFu; /* everything analog */
    board_init();
    EXPECT_UEQ(GPIOA->MODER, 0xABFFF7FFu); /* PA5 = 01, the rest untouched */
    EXPECT_UEQ(GPIOC->MODER, 0xF3FFFFFFu); /* PC13 = 00, the rest untouched */
}

TEST(board_init_from_all_zero_makes_pa5_an_output) {
    GPIOA->MODER = 0;
    GPIOC->MODER = 0;
    board_init();
    EXPECT_UEQ(GPIOA->MODER, 1u << 10);
    EXPECT_UEQ(GPIOC->MODER, 0u);
}

TEST(led_on_writes_the_set_half_of_bsrr_and_not_odr) {
    GPIOA->ODR = 0x0000F00Fu; /* other pins are high and an ISR owns them */
    led_on();
    EXPECT_UEQ(GPIOA->ODR, 0x0000F00Fu); /* you didn't touch ODR yourself */
    EXPECT_UEQ(GPIOA->BSRR, 1u << 5);
    sim_gpio_apply_bsrr(GPIOA);
    EXPECT_UEQ(GPIOA->ODR, 0x0000F02Fu);
}

TEST(led_off_writes_the_reset_half_of_bsrr) {
    GPIOA->ODR = 0x0000FFFFu;
    led_off();
    EXPECT_UEQ(GPIOA->ODR, 0x0000FFFFu);
    EXPECT_UEQ(GPIOA->BSRR, 1u << 21);
    sim_gpio_apply_bsrr(GPIOA);
    EXPECT_UEQ(GPIOA->ODR, 0x0000FFDFu);
}

TEST(led_blinks_on_off_on) {
    board_init();
    led_on();
    sim_gpio_apply_bsrr(GPIOA);
    EXPECT_TRUE(GPIOA->ODR & (1u << 5));
    led_off();
    sim_gpio_apply_bsrr(GPIOA);
    EXPECT_FALSE(GPIOA->ODR & (1u << 5));
    led_on();
    sim_gpio_apply_bsrr(GPIOA);
    EXPECT_TRUE(GPIOA->ODR & (1u << 5));
}

TEST(button_is_active_low_and_ignores_other_pins) {
    board_init();
    GPIOC->IDR = 0xFFFFFFFFu; /* idle: pull-up holds PC13 high, other pins noisy */
    EXPECT_FALSE(button_pressed());
    sim_gpio_set_input(GPIOC, 13, false); /* press */
    EXPECT_TRUE(button_pressed());
    GPIOC->IDR = 0x00000000u;             /* everything low, PC13 included */
    EXPECT_TRUE(button_pressed());
    sim_gpio_set_input(GPIOC, 13, true);  /* release, other pins still low */
    EXPECT_FALSE(button_pressed());
}
