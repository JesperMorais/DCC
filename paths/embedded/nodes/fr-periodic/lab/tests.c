#include "FreeRTOS.h"
#include "sim.h"
#include "task.h"

TEST(every_cycle_starts_exactly_on_the_10ms_grid) {
    app_main();
    sim_run(200);
    for (uint32_t k = 0; k < 20; k++) EXPECT_EQ(g_cycle_start[k], k * 10);
}

TEST(hundred_cycles_per_second_under_cpu_load) {
    app_main();
    sim_run(1000);
    EXPECT_EQ(g_cycles, 100);
    /* diagnostics gets everything control doesn't use: 1000 - 100 * 2 */
    EXPECT_EQ(sim_ran_ticks("diagnostics"), 800);
}

TEST(no_overruns_when_every_cycle_fits) {
    app_main();
    sim_run(1000);
    EXPECT_EQ(g_overruns, 0);
}

TEST(one_slow_cycle_is_counted_as_one_overrun) {
    g_slow_cycle = 3; /* starts at 30, runs 14 ticks to 44: past the 40 deadline */
    app_main();
    sim_run(200);
    EXPECT_EQ(g_overruns, 1);
    EXPECT_EQ(g_cycle_start[4], 44); /* the late cycle starts right away... */
    EXPECT_EQ(g_cycle_start[5], 50); /* ...and the loop is back on the grid */
    EXPECT_EQ(g_cycle_start[9], 90);
}

TEST(a_long_stall_overruns_twice_then_catches_up) {
    g_slow_cycle = 3;
    g_slow_ticks = 25; /* 30..55: misses the 40 AND the 50 deadline */
    app_main();
    sim_run(200);
    EXPECT_EQ(g_overruns, 2);
    EXPECT_EQ(g_cycle_start[4], 55);
    EXPECT_EQ(g_cycle_start[5], 57); /* back-to-back catch-up cycle */
    EXPECT_EQ(g_cycle_start[6], 60);
}
