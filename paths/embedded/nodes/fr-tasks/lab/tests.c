#include <string.h>

#include "FreeRTOS.h"
#include "sim.h"
#include "task.h"

TEST(app_main_creates_the_tasks_and_returns_their_handles) {
    EXPECT_EQ(app_main(), pdPASS);
    EXPECT_NOT_NULL(g_estop_handle);
    EXPECT_NOT_NULL(g_sensor_handle);
    EXPECT_NOT_NULL(g_logger_handle);
    EXPECT_STR_EQ(pcTaskGetName(g_estop_handle), "estop");
    EXPECT_STR_EQ(pcTaskGetName(g_sensor_handle), "sensor");
    EXPECT_STR_EQ(pcTaskGetName(g_logger_handle), "logger");
}

TEST(priorities_fit_freertos_and_keep_the_design_order) {
    EXPECT_EQ(app_main(), pdPASS);
    long estop = (long)uxTaskPriorityGet(g_estop_handle);
    long sensor = (long)uxTaskPriorityGet(g_sensor_handle);
    long logger = (long)uxTaskPriorityGet(g_logger_handle);
    EXPECT_TRUE(logger > (long)tskIDLE_PRIORITY); /* 0 is the idle task's */
    EXPECT_TRUE(sensor > logger);
    EXPECT_TRUE(estop > sensor);
    EXPECT_TRUE(estop < configMAX_PRIORITIES);
    EXPECT_TRUE(sim_max_priority_seen("boot") > estop); /* boot outranks everyone */
}

TEST(boot_runs_first_then_deletes_itself) {
    EXPECT_EQ(app_main(), pdPASS);
    sim_run(20);
    EXPECT_STR_EQ(sim_running_at(0), "boot");
    EXPECT_STR_EQ(sim_running_at(2), "boot");
    EXPECT_TRUE(g_boot_done);
    EXPECT_TRUE(sim_task_finished("boot"));
    EXPECT_EQ(sim_first_run_at_or_after("estop", 0), 3); /* nothing else ran during boot */
}

TEST(sensor_reads_its_period_from_pvParameters) {
    g_sensor_cfg.period_ms = 30; /* reconfigure before the task is created */
    EXPECT_EQ(app_main(), pdPASS);
    sim_run(100);
    /* runs at ~4, ~36, ~68 with a 30 ms period (it would be ~8 samples at 10 ms) */
    EXPECT_EQ(g_sensor_cfg.samples, 3);
}

TEST(estop_is_never_held_up_by_lower_priority_work) {
    EXPECT_EQ(app_main(), pdPASS);
    sim_run(100);
    /* 1 tick of work + 5 ticks of delay, starting right after boot (tick 3): 3, 9, 15, ... 99 */
    EXPECT_EQ(g_estop_polls, 17);
    EXPECT_TRUE(g_log_lines >= 3); /* the logger still gets the gaps */
}
