#include <stdbool.h>
#include <stdint.h>
#include "mcu.h"

/* PC13 is active-low: pressed = 0 V */
static void level(bool pressed, unsigned ms) {
    sim_gpio_set_input(GPIOC, 13, !pressed);
    sim_systick(ms);
}

/* contact chatter: alternate every 1-3 ms for about `ms` ms, ending on `final` */
static void chatter(unsigned ms, bool final) {
    unsigned t = 0, k = 0;
    bool lvl = final;
    while (t + 3 <= ms) {
        lvl = !lvl;
        unsigned step = 1 + (k++ % 3);
        level(lvl, step);
        t += step;
    }
    sim_gpio_set_input(GPIOC, 13, !final);
}

static void start(void) {
    GPIOC->MODER = 0xFFFFFFFFu;
    sim_gpio_set_input(GPIOC, 13, true); /* released */
    button_init();
}

TEST(init_makes_pc13_an_input_and_an_idle_line_does_nothing) {
    start();
    EXPECT_UEQ(GPIOC->MODER, 0xF3FFFFFFu);
    level(false, 500);
    EXPECT_FALSE(button_is_pressed());
    EXPECT_EQ(button_short_presses(), 0);
    EXPECT_EQ(button_long_presses(), 0);
}

TEST(a_clean_tap_is_one_short_press_after_the_release) {
    start();
    level(true, 100);
    EXPECT_TRUE(button_is_pressed());
    EXPECT_EQ(button_short_presses(), 0); /* counted on release, not on press */
    level(false, 100);
    EXPECT_FALSE(button_is_pressed());
    EXPECT_EQ(button_short_presses(), 1);
    EXPECT_EQ(button_long_presses(), 0);
}

TEST(a_press_is_accepted_only_after_about_20_ms_of_stable_level) {
    start();
    level(true, 15);
    EXPECT_FALSE(button_is_pressed()); /* not yet stable for 20 ms */
    level(true, 10);
    EXPECT_TRUE(button_is_pressed());  /* 25 ms: it is now */
    level(false, 15);
    EXPECT_TRUE(button_is_pressed());  /* the release must be stable too */
    level(false, 10);
    EXPECT_FALSE(button_is_pressed());
}

TEST(bouncy_press_and_bouncy_release_count_once) {
    start();
    for (int i = 0; i < 3; i++) {
        chatter(12, true);   /* the contacts hit and spring back */
        level(true, 200);
        chatter(12, false);  /* and again on the way up */
        level(false, 200);
    }
    EXPECT_EQ(button_short_presses(), 3);
    EXPECT_EQ(button_long_presses(), 0);
}

TEST(glitches_shorter_than_the_debounce_window_are_ignored) {
    start();
    for (int i = 0; i < 10; i++) {
        level(true, 1 + (unsigned)(i % 5) * 3); /* 1..13 ms EMI spikes */
        level(false, 30);
    }
    level(true, 17);   /* nearly long enough... */
    level(false, 40);
    /* a glitch that releases for 2 ms in the middle of a hold doesn't split it */
    level(true, 100);
    level(false, 2);
    level(true, 100);
    level(false, 60);
    EXPECT_EQ(button_short_presses(), 1);
    EXPECT_EQ(button_long_presses(), 0);
}

TEST(a_long_hold_fires_exactly_one_long_press_while_still_held) {
    start();
    level(true, 20 + 950);
    EXPECT_EQ(button_long_presses(), 0);   /* not before about 1 s */
    level(true, 100);
    EXPECT_EQ(button_long_presses(), 1);   /* fired while the finger is still down */
    level(true, 3000);
    EXPECT_EQ(button_long_presses(), 1);   /* and never again for the same hold */
    EXPECT_TRUE(button_is_pressed());
    chatter(10, false);
    level(false, 100);
    EXPECT_EQ(button_long_presses(), 1);
    EXPECT_EQ(button_short_presses(), 0);  /* releasing a long press is not a tap */
}

TEST(the_fsm_returns_to_idle_ready_for_the_next_gesture) {
    start();
    level(true, 1500);  /* long */
    level(false, 100);
    level(true, 300);   /* tap */
    level(false, 100);
    level(true, 1200);  /* long again */
    level(false, 100);
    level(true, 50);    /* tap */
    level(false, 100);
    EXPECT_EQ(button_long_presses(), 2);
    EXPECT_EQ(button_short_presses(), 2);
    EXPECT_FALSE(button_is_pressed());
}
