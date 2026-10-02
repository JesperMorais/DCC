#include <zephyr/kernel.h>

/* The IMU interrupts every 4 ms from tick 4 up to and including `last`. */
static uint32_t imu_last;
static void imu_line(void) {
    if (k_uptime_get_32() <= imu_last) imu_isr();
}
static void imu_until(uint32_t last) {
    imu_last = last;
    sim_irq_every(4, 4, "imu", imu_line);
}

static int32_t expected_sum(uint32_t n) {
    int32_t s = 0;
    for (uint32_t seq = 1; seq <= n; seq++) s += imu_value(seq);
    return s;
}

TEST(no_message_is_lost_under_load) {
    imu_until(990);
    sim_run(1050);
    EXPECT_EQ(isr_produced, 247);
    EXPECT_EQ(isr_drops, 0);
    EXPECT_EQ(stats.processed, 247);
    EXPECT_EQ(stats.sum, expected_sum(247));
}

TEST(reports_every_100ms_exactly) {
    imu_until(2000);
    sim_run(1050);
    EXPECT_EQ(n_reports, 10);
    for (uint32_t i = 0; i < 10; i++) EXPECT_EQ(reports[i].t_ms, 100 * (i + 1));
}

TEST(reporting_runs_on_the_system_workqueue) {
    imu_until(2000);
    sim_run(1050);
    EXPECT_EQ(sim_ran_ticks("sysworkq"), 10 * REPORT_US / 1000);
}

TEST(reports_are_consistent_snapshots) {
    imu_until(2000);
    sim_run(1050);
    EXPECT_EQ(n_reports, 10);
    for (uint32_t i = 0; i < n_reports; i++) {
        /* count and sum taken at the same moment, and messages are processed in order */
        EXPECT_EQ(reports[i].sum, expected_sum(reports[i].processed));
        if (i) EXPECT_TRUE(reports[i].processed > reports[i - 1].processed);
    }
}

TEST(no_priority_inversion_stalls_processing) {
    imu_until(2000);
    sim_run(1050);
    /* the logger holds stats_lock while BLE bursts start: processing must not wait out the burst */
    EXPECT_TRUE(stats.processed >= 245);
    EXPECT_EQ(isr_drops, 0);
    EXPECT_TRUE(stats.max_latency_ms <= 10);
}

TEST(everyone_else_still_gets_their_cpu) {
    imu_until(2000);
    sim_run(1000);
    EXPECT_TRUE(sim_ran_ticks("ble") >= 240);
    EXPECT_TRUE(flash_writes >= 15);
}
