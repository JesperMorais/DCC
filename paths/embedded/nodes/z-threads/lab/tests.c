#include <zephyr/kernel.h>

/* The radio wakes at 9, 59, 109, ... and its 3 ms burst overlaps control's
 * releases at 10, 60, 110, ... That collision is the interesting part. */

TEST(control_starts_every_period_within_3ms) {
    sim_run(300);
    for (uint32_t k = 0; k < 29; k++) {
        uint32_t release = k * 10;
        uint32_t started = sim_first_run_at_or_after("control", release);
        if (started - release > 3) sim_mark("control was late here");
        EXPECT_TRUE(started - release <= 3);
    }
    EXPECT_TRUE(control_runs >= 29);
}

TEST(control_is_on_time_when_nothing_collides) {
    sim_run(100);
    /* 20, 30, 40: no radio burst nearby, so control must start on the dot */
    EXPECT_EQ(sim_first_run_at_or_after("control", 20), 20);
    EXPECT_EQ(sim_first_run_at_or_after("control", 30), 30);
    EXPECT_EQ(sim_first_run_at_or_after("control", 40), 40);
}

TEST(radio_tx_burst_is_never_interleaved) {
    sim_run(200);
    for (uint32_t burst = 9; burst < 200; burst += 50) {
        EXPECT_STR_EQ(sim_running_at(burst), "radio");
        EXPECT_STR_EQ(sim_running_at(burst + 1), "radio");
        EXPECT_STR_EQ(sim_running_at(burst + 2), "radio");
    }
    EXPECT_EQ(radio_bursts, 4);
}

TEST(logger_still_gets_its_flash_time) {
    sim_run(500);
    /* control uses ~10% and the radio ~6%: the logger should get most of the rest */
    EXPECT_TRUE(sim_ran_ticks("logger") >= 350);
    EXPECT_TRUE(flash_pages >= 14);
}

TEST(priorities_say_what_you_mean) {
    sim_run(1);
    /* lower number = more urgent in Zephyr */
    EXPECT_TRUE(k_thread_priority_get(control) < k_thread_priority_get(logger));
    /* long flash writes must be preemptible (>= 0) */
    EXPECT_TRUE(k_thread_priority_get(logger) >= 0);
}
