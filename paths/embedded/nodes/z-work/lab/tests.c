#include <zephyr/kernel.h>

static void press_a(rtos_tick_t t, int n) { while (n--) sim_irq_at(t, "button A", button_a_isr); }
static void press_b(rtos_tick_t t, int n) { while (n--) sim_irq_at(t, "button B", button_b_isr); }

TEST(one_press_is_handled_right_away) {
    press_a(5, 1);
    sim_run(20);
    EXPECT_STR_EQ(event_log, "A");
    EXPECT_EQ(event_at[0], 5);
    /* the system workqueue is cooperative (-1): it takes the CPU from ui at once */
    EXPECT_EQ(sim_first_run_at_or_after("sysworkq", 5), 5);
}

TEST(a_burst_of_presses_is_not_lost) {
    press_a(10, 3); /* three edges inside one tick: k_work_submit says "already pending" twice */
    press_a(11, 1); /* and one more while the handler is busy */
    sim_run(40);
    EXPECT_STR_EQ(event_log, "AAAA");
    EXPECT_EQ(btn_a.handled, 4);
}

TEST(one_handler_serves_both_buttons) {
    press_a(20, 2);
    press_b(20, 3);
    sim_run(50);
    EXPECT_STR_EQ(event_log, "AABBB");
    EXPECT_EQ(btn_b.handled, 3);
    EXPECT_TRUE(btn_a.work.handler == btn_b.work.handler);
}

TEST(the_work_runs_in_thread_context) {
    press_a(10, 2);
    press_b(30, 3);
    sim_run(60);
    EXPECT_EQ(n_events, 5);
    EXPECT_EQ(sim_ran_ticks("sysworkq"), 5 * HANDLE_US / 1000);
}

TEST(never_block_the_system_workqueue) {
    press_a(10, 1);
    press_b(11, 1);
    sim_run(40);
    EXPECT_STR_EQ(event_log, "AB");
    EXPECT_EQ(event_at[1], 12); /* B starts the moment A's 2 ms are done */
}
