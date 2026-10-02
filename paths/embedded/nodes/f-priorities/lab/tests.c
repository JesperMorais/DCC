#include <stddef.h>
#include <string.h>
#include "rtos.h"
#include "sim.h"

/* Grading from the timeline: job k of a task is released at k*period and must get its
 * full WCET of CPU before (k+1)*period. Returns the release tick of the first job that
 * missed its deadline, or -1 if every job made it. */
static long first_miss(size_t task, rtos_tick_t horizon) {
    const struct task_spec *t = &gimbal_tasks[task];
    for (rtos_tick_t r = 0; r + t->period <= horizon; r += t->period) {
        rtos_tick_t got = 0;
        for (rtos_tick_t i = r; i < r + t->period; i++)
            if (strcmp(sim_running_at(i), t->name) == 0) got++;
        if (got < t->wcet) return (long)r;
    }
    return -1;
}

/* Response time of the job released at r: last tick it ran, + 1, - r. */
static rtos_tick_t worst_response(size_t task, rtos_tick_t horizon) {
    const struct task_spec *t = &gimbal_tasks[task];
    rtos_tick_t worst = 0;
    for (rtos_tick_t r = 0; r + t->period <= horizon; r += t->period)
        for (rtos_tick_t i = r; i < r + t->period; i++)
            if (strcmp(sim_running_at(i), t->name) == 0 && i + 1 - r > worst) worst = i + 1 - r;
    return worst;
}

TEST(motor_never_misses_a_deadline) {
    gimbal_start();
    sim_run(1000);
    EXPECT_EQ(first_miss(0, 1000), -1); /* -1 = no miss; otherwise the release tick of the first late job */
}

TEST(imu_never_misses_a_deadline) {
    gimbal_start();
    sim_run(1000);
    EXPECT_EQ(first_miss(1, 1000), -1);
}

TEST(telemetry_never_misses_a_deadline) {
    gimbal_start();
    sim_run(1000);
    EXPECT_EQ(first_miss(2, 1000), -1);
}

TEST(priorities_are_rate_monotonic) {
    EXPECT_TRUE(PRIO_MOTOR > PRIO_IMU);
    EXPECT_TRUE(PRIO_IMU > PRIO_TELEMETRY);
}

TEST(utilisation_of_the_gimbal_set_is_80_percent) {
    EXPECT_NEAR(utilisation(gimbal_tasks, gimbal_task_count), 0.80, 1e-9);
}

TEST(utilisation_of_other_sets) {
    const struct task_spec two[] = {{"a", 4, 1, 2}, {"b", 8, 3, 1}};
    EXPECT_NEAR(utilisation(two, 2), 0.625, 1e-9);
    EXPECT_NEAR(utilisation(two, 0), 0.0, 1e-9);
}

TEST(response_times_match_the_analysis) {
    gimbal_start();
    sim_run(1000);
    EXPECT_EQ(worst_response(0, 1000), 1);  /* highest priority: nothing ever gets in its way */
    EXPECT_EQ(worst_response(1, 1000), 4);  /* 3 + one motor job */
    EXPECT_EQ(worst_response(2, 1000), 15); /* 6 + 3 motor jobs + 2 imu jobs */
}
