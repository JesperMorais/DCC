#include "rtos.h"
#include "sim.h"

/* The display task (another team's code) keeps the CPU busy at priority 2. */
static void display_task(void *arg) {
    (void)arg;
    for (;;) rtos_busy(1);
}

static void burst_mark_isr(void) { sim_mark("burst of CAN frames"); }

static void board_start(void) {
    rtos_task_create("display", display_task, NULL, 2);
    can_start();
}

TEST(one_frame_is_parsed_by_the_task) {
    board_start();
    sim_irq_at(10, "can_rx_irq", can_rx_isr);
    sim_run(30);
    EXPECT_EQ(frames_handled, 1);
    EXPECT_EQ(sim_ran_ticks("can_rx"), 2); /* the task did the parsing, not the ISR */
}

TEST(handler_runs_on_the_irq_tick) {
    board_start();
    sim_irq_at(10, "can_rx_irq", can_rx_isr);
    sim_irq_at(37, "can_rx_irq", can_rx_isr);
    sim_irq_at(55, "can_rx_irq", can_rx_isr);
    sim_run(70);
    EXPECT_EQ(sim_first_run_at_or_after("can_rx", 10), 10);
    EXPECT_EQ(sim_first_run_at_or_after("can_rx", 37), 37);
    EXPECT_EQ(sim_first_run_at_or_after("can_rx", 55), 55);
    EXPECT_EQ(frames_handled, 3);
}

TEST(steady_stream_for_1000_ticks_zero_latency) {
    board_start();
    sim_irq_every(5, 20, "can_rx_irq", can_rx_isr);
    sim_run(1000);
    EXPECT_EQ(frames_handled, 50);
    for (rtos_tick_t t = 5; t < 1000; t += 20) EXPECT_EQ(sim_first_run_at_or_after("can_rx", t), t);
}

TEST(burst_of_5_back_to_back_frames_loses_nothing) {
    board_start();
    sim_irq_at(100, "burst", burst_mark_isr); /* 5 frames in 5 ticks, each needs 2 ticks to parse */
    for (rtos_tick_t t = 100; t < 105; t++) sim_irq_at(t, "can_rx_irq", can_rx_isr);
    sim_run(130);
    EXPECT_EQ(frames_handled, 5);
}

TEST(burst_of_12_frames_on_one_tick_loses_nothing) {
    board_start();
    sim_irq_at(100, "burst", burst_mark_isr); /* 12 frames arrive at once */
    for (int i = 0; i < 12; i++) sim_irq_at(100, "can_rx_irq", can_rx_isr);
    sim_run(125); /* 12 frames x 2 ticks = done by tick 124 */
    EXPECT_EQ(frames_handled, 12);
    EXPECT_EQ(rtos_sem_count(rx_sem), 0);
}
