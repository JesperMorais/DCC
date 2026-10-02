#include <stdint.h>
#include "rtos.h"
#include "sim.h"

/* ---- the board: sample n (1, 2, 3, ...) reads 10*n, so averages are easy to check ---- */
static unsigned nsamples;
static int32_t logged[300];
static int nlogged;
static rtos_tick_t log_cost = 3; /* ticks per SD-card line */

uint16_t adc_read(void) {
    nsamples++;
    return (uint16_t)(10 * nsamples);
}

void log_line(int32_t avg) {
    if (nlogged < 300) logged[nlogged] = avg;
    nlogged++;
    rtos_busy(log_cost);
}

/* A firmware update writing flash at priority 5 hogs the CPU for ticks 0..59. */
static void flash_writer_task(void *arg) {
    (void)arg;
    sim_mark("flash write: everyone else starves for 60 ticks");
    rtos_busy(60);
}

TEST(averages_arrive_in_order) {
    station_start();
    sim_irq_every(5, 5, "adc", adc_isr); /* a sample every 5 ticks */
    sim_run(110);
    EXPECT_TRUE(nlogged >= 5);
    const int expected[] = {25, 65, 105, 145, 185}; /* (10+20+30+40)/4, (50+60+70+80)/4, ... */
    for (int i = 0; i < 5; i++) EXPECT_EQ(logged[i], expected[i]);
}

TEST(nothing_lost_at_the_normal_rate_for_1000_ticks) {
    station_start();
    sim_irq_every(5, 5, "adc", adc_isr);
    sim_run(1010); /* samples at 5, 10, ..., 1010: 202 of them, 50 full windows */
    EXPECT_EQ(samples_dropped, 0);
    EXPECT_EQ(nlogged, 50);
    EXPECT_EQ(logged[49], 1985); /* the average of samples 197..200 = (1970+1980+1990+2000)/4 */
}

TEST(a_full_queue_drops_the_newest_samples_and_counts_them) {
    rtos_task_create("flash_writer", flash_writer_task, NULL, 5);
    station_start();
    sim_irq_every(5, 5, "adc", adc_isr);
    sim_run(100);
    /* Samples 1..8 (ticks 5..40) fill the queue; 9..12 (ticks 45..60) find it full. */
    EXPECT_EQ(samples_dropped, 4);
    EXPECT_EQ(logged[0], 25);  /* samples 1..4 */
    EXPECT_EQ(logged[1], 65);  /* samples 5..8 */
    EXPECT_EQ(logged[2], 145); /* samples 13..16: 9..12 were dropped */
}

TEST(a_slow_logger_causes_back_pressure_not_chaos) {
    log_cost = 30; /* the SD card is having a bad day: 30 ticks per line */
    station_start();
    sim_irq_every(5, 5, "adc", adc_isr);
    sim_run(600);
    EXPECT_FALSE(sim_deadlocked());
    EXPECT_TRUE(samples_dropped > 0);      /* the source is faster than the sink: something has to give */
    EXPECT_TRUE(nlogged >= 19);            /* the logger works flat out: 600 / 30 lines */
    for (int i = 1; i < nlogged && i < 300; i++) EXPECT_TRUE(logged[i] > logged[i - 1]); /* still in order */
    EXPECT_EQ(logged[0], 25);
}

TEST(the_filter_preempts_a_slow_logger) {
    log_cost = 22; /* the logger is mid-write whenever a window completes */
    station_start();
    sim_irq_every(5, 5, "adc", adc_isr);
    sim_run(400);
    /* Every 4th sample (ticks 20, 40, ...) completes a window: the filter runs on that very tick. */
    for (rtos_tick_t t = 20; t < 400; t += 20) EXPECT_EQ(sim_first_run_at_or_after("filter", t), t);
    EXPECT_EQ(samples_dropped, 0);
}
