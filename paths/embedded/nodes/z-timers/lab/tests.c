#include <zephyr/kernel.h>

TEST(samples_every_100ms_on_the_dot) {
    sim_run(1050);
    EXPECT_EQ(samples, 10);
    for (uint32_t i = 0; i < 10; i++) EXPECT_EQ(sample_at[i], 100 * (i + 1));
}

TEST(sampling_happens_in_the_workqueue) {
    sim_run(1050);
    /* 10 samples x 3 ms of I2C, all on the system workqueue thread */
    EXPECT_EQ(sim_ran_ticks("sysworkq"), 30);
}

TEST(a_stalled_workqueue_counts_missed_periods) {
    sim_irq_at(250, "fw update", fw_isr); /* coop thread hogs the CPU from 250 to 470 */
    sim_run(1050);
    /* the expiries at 300 and 400 collapse into one late run at 470 */
    EXPECT_EQ(sample_at[2], 470);
    EXPECT_EQ(sample_at[3], 500); /* ...and the grid is untouched */
    EXPECT_EQ(missed, 1);
    EXPECT_EQ(samples + missed, 10);
}

TEST(backlight_turns_off_after_500ms_idle) {
    sim_run(499);
    EXPECT_TRUE(backlight_on);
    sim_run(2);
    EXPECT_FALSE(backlight_on);
    EXPECT_EQ(backlight_off_at, 500);
    EXPECT_EQ(backlight_offs, 1);
}

TEST(each_touch_restarts_the_timeout) {
    sim_irq_at(300, "touch", touch_isr);
    sim_irq_at(700, "touch", touch_isr);
    sim_run(1199);
    EXPECT_TRUE(backlight_on); /* not off at 500, not off at 800 */
    EXPECT_EQ(backlight_offs, 0);
    sim_run(2);
    EXPECT_FALSE(backlight_on);
    EXPECT_EQ(backlight_off_at, 1200);
    EXPECT_EQ(backlight_offs, 1);
}

TEST(a_touch_after_the_timeout_wakes_the_backlight) {
    sim_irq_at(600, "touch", touch_isr);
    sim_run(601);
    EXPECT_TRUE(backlight_on);
    EXPECT_EQ(backlight_offs, 1);
    sim_run(500);
    EXPECT_FALSE(backlight_on);
    EXPECT_EQ(backlight_off_at, 1100);
    EXPECT_EQ(backlight_offs, 2);
}
