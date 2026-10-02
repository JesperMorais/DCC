#include <string.h>

#include "FreeRTOS.h"
#include "mcu.h"
#include "sim.h"
#include "task.h"

/* The "wire": each USART1 interrupt delivers the next byte of `feed`. */
static const char *feed;
static void usart1_rx(void) { sim_uart_receive((uint8_t)*feed++); }

static void bytes_at(const char *text, rtos_tick_t first, rtos_tick_t spacing) {
    feed = text;
    for (size_t i = 0; text[i]; i++) sim_irq_at(first + (rtos_tick_t)i * spacing, "USART1", usart1_rx);
}

TEST(decoder_runs_in_the_same_tick_as_the_interrupt) {
    app_main();
    bytes_at("A", 10, 0);
    sim_run(30);
    EXPECT_EQ(g_rx_count, 1);
    EXPECT_EQ(g_rx_tick[0], 10); /* one tick late means no portYIELD_FROM_ISR */
    EXPECT_STR_EQ(sim_running_at(10), "decoder");
}

TEST(every_byte_arrives_in_order_and_on_time) {
    app_main();
    bytes_at("HELLO", 5, 10);
    sim_run(60);
    EXPECT_STR_EQ(g_rx_buf, "HELLO");
    for (uint32_t i = 0; i < 5; i++) EXPECT_EQ(g_rx_tick[i], 5 + i * 10);
}

TEST(the_busy_ui_task_is_preempted_not_waited_for) {
    app_main();
    bytes_at("XYZ", 20, 7);
    sim_run(50);
    EXPECT_STR_EQ(sim_running_at(19), "ui");
    EXPECT_STR_EQ(sim_running_at(20), "decoder");
    EXPECT_STR_EQ(sim_running_at(27), "decoder");
    EXPECT_STR_EQ(sim_running_at(34), "decoder");
}

TEST(a_burst_in_one_tick_is_queued_not_lost) {
    app_main();
    bytes_at("123", 20, 0); /* three interrupts at the same tick */
    sim_run(40);
    EXPECT_STR_EQ(g_rx_buf, "123");
    EXPECT_EQ(g_rx_tick[0], 20);
    EXPECT_EQ(g_rx_tick[2], 22);
    EXPECT_EQ(g_rx_dropped, 0);
}

TEST(a_full_queue_drops_and_counts_instead_of_blocking) {
    app_main();
    bytes_at("0123456789", 30, 0); /* 10 bytes, queue holds 8 */
    sim_run(60);
    /* 8 queued (+1 if the decoder already took the first one off): the rest are dropped */
    EXPECT_EQ(g_rx_count + g_rx_dropped, 10);
    EXPECT_TRUE(g_rx_dropped >= 1 && g_rx_dropped <= 2);
    EXPECT_EQ(strncmp(g_rx_buf, "01234567", 8), 0);
}
