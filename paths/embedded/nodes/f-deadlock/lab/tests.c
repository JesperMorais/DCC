#include "rtos.h"
#include "sim.h"

TEST(no_deadlock_with_the_default_phasing) {
    bike_start();
    sim_run(300);
    EXPECT_FALSE(sim_deadlocked());
}

TEST(both_tasks_finish_all_their_rounds) {
    bike_start();
    sim_run(300);
    EXPECT_TRUE(sim_task_finished("nav"));
    EXPECT_TRUE(sim_task_finished("archive"));
    EXPECT_EQ(fixes_logged, ROUNDS);
    EXPECT_EQ(files_archived, ROUNDS);
}

TEST(no_deadlock_when_nav_starts_at_tick_0) {
    nav_start_delay = 0;
    bike_start();
    sim_run(300);
    EXPECT_FALSE(sim_deadlocked());
    EXPECT_TRUE(sim_task_finished("nav") && sim_task_finished("archive"));
}

TEST(no_deadlock_when_nav_starts_at_tick_2) {
    nav_start_delay = 2;
    bike_start();
    sim_run(300);
    EXPECT_FALSE(sim_deadlocked());
    EXPECT_TRUE(sim_task_finished("nav") && sim_task_finished("archive"));
}

TEST(no_deadlock_when_nav_starts_at_tick_3) {
    nav_start_delay = 3;
    bike_start();
    sim_run(300);
    EXPECT_FALSE(sim_deadlocked());
    EXPECT_TRUE(sim_task_finished("nav") && sim_task_finished("archive"));
}

TEST(both_locks_are_free_at_the_end) {
    bike_start();
    sim_run(300);
    EXPECT_NULL(rtos_mutex_owner(gps_lock));
    EXPECT_NULL(rtos_mutex_owner(sd_lock));
}
