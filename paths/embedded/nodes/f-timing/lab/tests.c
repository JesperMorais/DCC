#include <stdint.h>
#include "rtos.h"
#include "sim.h"

/* ---- the "board": adc_read() logs WHEN each sample was taken ---- */
#define MAX_SAMPLES 200
static rtos_tick_t sample_at[MAX_SAMPLES];
static int nsamples;
static uint16_t script[MAX_SAMPLES]; /* readings to return; 0 means a normal 1000 */

uint16_t adc_read(void) {
    uint16_t r = script[nsamples < MAX_SAMPLES ? nsamples : 0];
    if (nsamples < MAX_SAMPLES) sample_at[nsamples] = rtos_now();
    nsamples++;
    if (r == 3500) sim_mark("spike: 6 ticks of filtering");
    if (r == 0xFFFF) sim_mark("sensor fault: 13 ticks, overrun!");
    return r ? r : 1000;
}

TEST(first_samples_land_on_the_10_tick_grid) {
    sampler_start();
    sim_run(45);
    EXPECT_EQ(nsamples, 5);
    for (int i = 0; i < 5; i++) EXPECT_EQ(sample_at[i], i * 10);
}

TEST(a_spike_does_not_shift_the_next_sample) {
    script[3] = 3500; /* the sample at tick 30 takes 6 ticks to filter */
    sampler_start();
    sim_run(65);
    EXPECT_EQ(sample_at[3], 30);
    EXPECT_EQ(sample_at[4], 40);
    EXPECT_EQ(sample_at[6], 60);
}

TEST(no_drift_over_1000_ticks_with_varying_work) {
    for (int i = 0; i < MAX_SAMPLES; i += 7) script[i] = 3500; /* a spike every 7th sample */
    sampler_start();
    sim_run(1000);
    EXPECT_EQ(nsamples, 100);
    for (int i = 0; i < 100; i++) EXPECT_EQ(sample_at[i], i * 10);
}

TEST(after_an_overrun_the_sampler_gets_back_on_the_grid) {
    script[2] = 0xFFFF; /* the sample at tick 20 takes 13 ticks: it runs past 30 */
    sampler_start();
    sim_run(75);
    EXPECT_EQ(sample_at[2], 20);
    EXPECT_EQ(sample_at[3], 33); /* late: released immediately, the period already passed */
    EXPECT_EQ(sample_at[4], 40); /* ...but the grid itself never moved */
    EXPECT_EQ(sample_at[7], 70);
}

/* A higher-priority radio task steals the CPU from tick 30 to 33. */
static void radio_task(void *arg) {
    (void)arg;
    rtos_delay(30);
    sim_mark("radio burst");
    rtos_busy(4);
}

TEST(preemption_causes_jitter_but_not_drift) {
    rtos_task_create("radio", radio_task, NULL, 5);
    sampler_start();
    sim_run(85);
    EXPECT_EQ(sample_at[3], 34); /* jitter: this one is 4 ticks late */
    EXPECT_EQ(sample_at[4], 40); /* no drift: the next one is on time */
    EXPECT_EQ(sample_at[8], 80);
}
