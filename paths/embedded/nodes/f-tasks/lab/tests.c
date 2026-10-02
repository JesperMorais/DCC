#include <stdbool.h>
#include <string.h>
#include "rtos.h"
#include "sim.h"

static void flash_done_isr(void) {
    flash_ready = true;
    sim_mark("flash ready");
}

/* the flash finishes a page at ticks 15, 55, 95, 135 and 175 */
static void boot(rtos_tick_t ticks) {
    sim_irq_every(15, 40, "flash_done", flash_done_isr);
    app_start();
    sim_run(ticks);
}

TEST(control_starts_on_every_multiple_of_10) {
    boot(200);
    for (rtos_tick_t k = 0; k < 20; k++) EXPECT_EQ(sim_first_run_at_or_after("control", k * 10), k * 10);
    EXPECT_EQ(control_runs, 20);
}

TEST(priorities_follow_the_deadlines) {
    boot(1);
    int c = sim_max_priority_seen("control"), b = sim_max_priority_seen("blink"), l = sim_max_priority_seen("logger");
    EXPECT_TRUE(c > b); /* 10-tick deadline beats 50-tick deadline */
    EXPECT_TRUE(b > l); /* and both beat "whenever" */
    EXPECT_TRUE(l >= 1);
    EXPECT_TRUE(c <= 5);
}

TEST(blink_runs_within_2_ticks_of_each_release) {
    boot(250);
    for (rtos_tick_t k = 0; k < 5; k++) {
        rtos_tick_t at = sim_first_run_at_or_after("blink", k * 50);
        EXPECT_TRUE(at <= k * 50 + 2);
    }
    EXPECT_EQ(blinks, 5);
}

TEST(every_flash_ready_event_gets_logged_promptly) {
    boot(200);
    for (rtos_tick_t e = 15; e < 200; e += 40) EXPECT_TRUE(sim_first_run_at_or_after("logger", e) <= e + 2);
    EXPECT_EQ(logs_written, 5);
}

TEST(the_logger_uses_cpu_only_for_real_work_not_for_waiting) {
    boot(200);
    EXPECT_EQ(logs_written, 5);
    EXPECT_EQ(sim_ran_ticks("logger"), 5 * 3); /* 3 ticks per entry, zero while waiting */
}

TEST(the_cpu_is_idle_most_of_the_time) {
    boot(200);
    int idle = 0;
    for (rtos_tick_t t = 0; t < 200; t++) idle += strcmp(sim_running_at(t), "idle") == 0;
    /* control 40 + blink 4 + logger 15 busy ticks: the other ~140 can be spent asleep */
    EXPECT_TRUE(idle >= 120);
}
