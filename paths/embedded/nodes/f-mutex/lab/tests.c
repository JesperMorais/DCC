#include "rtos.h"
#include "sim.h"

/* ---- the board ---- */
static int bus_violations, late_jobs;
static rtos_tick_t response[200]; /* response[k] = response time of the attitude job released at 2 + 10k */
static int attitude_jobs;

void spi_transfer(rtos_tick_t ticks) {
    if (rtos_mutex_owner(spi_bus) != rtos_self()) {
        if (bus_violations++ == 0) sim_mark("SPI transfer without owning spi_bus!");
    }
    rtos_busy(ticks);
}

void attitude_done(rtos_tick_t release) {
    rtos_tick_t r = rtos_now() - release;
    if (attitude_jobs < 200) response[attitude_jobs] = r;
    attitude_jobs++;
    if (r > ATTITUDE_DEADLINE && late_jobs++ == 0) sim_mark("attitude MISSED its deadline");
}

TEST(attitude_meets_every_deadline_for_1000_ticks) {
    rover_start();
    sim_run(1000);
    EXPECT_EQ(late_jobs, 0);
    EXPECT_EQ(attitude_jobs, 100);
}

TEST(inversion_at_tick_12_is_bounded) {
    /* At 12, attitude wants the bus while telemetry is mid-transfer, and compress is ready. */
    rover_start();
    sim_run(40);
    EXPECT_EQ(response[1], 3); /* waits 1 tick for telemetry to finish its transfer, then 2 of its own */
}

TEST(telemetry_inherits_attitudes_priority_while_it_blocks_it) {
    rover_start();
    sim_run(1000);
    EXPECT_EQ(sim_max_priority_seen("telemetry"), 3);
    EXPECT_EQ(sim_max_priority_seen("compress"), 2);
}

TEST(formatting_happens_outside_the_critical_section) {
    /* At 2, telemetry is still formatting its packet. That must not keep attitude off the bus. */
    rover_start();
    sim_run(40);
    EXPECT_EQ(response[0], 2);
}

TEST(every_transfer_holds_the_bus_and_every_packet_is_sent) {
    rover_start();
    sim_run(1000);
    EXPECT_EQ(bus_violations, 0);
    EXPECT_EQ(packets_sent, 50);
    EXPECT_EQ(sim_ran_ticks("compress"), 200); /* 20 jobs x 10 ticks: nobody starved */
}
