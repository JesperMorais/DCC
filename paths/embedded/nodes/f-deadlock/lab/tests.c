#include "rtos.h"
#include "sim.h"

/* ---- the board ---- */
static int appends, stamps, unprotected;

static void check_both_held(const char *what) {
    if (rtos_mutex_owner(gps_lock) != rtos_self() || rtos_mutex_owner(sd_lock) != rtos_self()) {
        if (unprotected++ == 0) sim_mark(what);
    }
    rtos_busy(1);
}

void track_append(void) {
    appends++;
    check_both_held("track_append() without holding BOTH gps_lock and sd_lock!");
}

void archive_stamp(void) {
    stamps++;
    check_both_held("archive_stamp() without holding BOTH gps_lock and sd_lock!");
}

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

TEST(every_append_and_stamp_holds_both_locks) {
    bike_start();
    sim_run(300);
    EXPECT_EQ(appends, ROUNDS);
    EXPECT_EQ(stamps, ROUNDS);
    EXPECT_EQ(unprotected, 0);
}
