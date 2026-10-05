#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include "mcu.h"
#include "rtos.h"
#include "sim.h"

/* ---- the sensor: a frame every 25 ms from tick 10, all 4 bytes inside one ms (115200 baud) ---- */
static int16_t default_temp(unsigned n) { return (int16_t)(40 + n % 7); } /* 4.0 .. 4.6 °C */
static int16_t (*temp_of)(unsigned n) = default_temp;
static unsigned frames_sent;
static unsigned frame_limit = UINT_MAX;

static void send_frame(int16_t t) {
    uint8_t msb = (uint8_t)((uint16_t)t >> 8), lsb = (uint8_t)t;
    sim_uart_receive(SYNC_BYTE);
    sim_uart_receive(msb);
    sim_uart_receive(lsb);
    sim_uart_receive(msb ^ lsb);
}

static void sensor_isr(void) {
    if (frames_sent >= frame_limit) return;
    frames_sent++;
    send_frame(temp_of(frames_sent));
}

static void start(void) {
    monitor_start();
    sim_irq_every(10, 25, "USART1", sensor_isr);
}

static bool alarm_on(void) { return GPIOB->ODR & (1u << ALARM_PIN); }

TEST(hardware_is_configured_without_touching_other_bits) {
    USART1->CR1 = USART_CR1_TE;        /* the debug console already set up the transmitter */
    GPIOB->MODER = 0x00000C03u;        /* PB0 is the heartbeat LED; PB5 comes out of reset as analog */
    GPIOB->ODR = (1u << 0) | (1u << ALARM_PIN);
    monitor_start();
    EXPECT_UEQ(USART1->CR1, USART_CR1_TE | USART_CR1_UE | USART_CR1_RE | USART_CR1_RXNEIE);
    EXPECT_UEQ(GPIOB->MODER, 0x00000403u); /* PB5 = 01 (output), PB0 kept */
    EXPECT_UEQ(GPIOB->ODR, 1u << 0);       /* PB5 low, heartbeat kept */
}

TEST(frames_are_decoded_the_tick_after_they_arrive_even_during_a_redraw) {
    start();
    sim_run(70); /* frames at 10, 35 and 60; the display redraws from 35 to 105 */
    EXPECT_EQ(g_stats.frames, 3);
    EXPECT_EQ(g_stats.latest, default_temp(3));
    EXPECT_EQ(g_stats.latest_tick, 61); /* 1 ms of calibration, then stored: the redraw didn't delay it */
    sim_run(930);
    EXPECT_EQ(g_stats.frames, 40);
    EXPECT_EQ(g_bytes_dropped, 0);
    EXPECT_EQ(g_bad_frames, 0);
}

static void noise_then_good(void) {
    sim_uart_receive(0x42); /* line noise before the first sync */
    sim_uart_receive(0x17);
    send_frame(55);
}
static void corrupt(void) {
    sim_uart_receive(SYNC_BYTE);
    sim_uart_receive(0x00);
    sim_uart_receive(99);
    sim_uart_receive(0x77); /* wrong checksum */
}
static void negative(void) { send_frame(-40); } /* -4.0 °C: FF D8 */
static void sync_lookalike(void) { send_frame(0x00A5); } /* A5 00 A5 A5: data bytes equal to SYNC */

TEST(noise_and_corrupt_frames_are_rejected_and_the_parser_stays_in_step) {
    monitor_start();
    sim_irq_at(5, "USART1", noise_then_good);
    sim_irq_at(7, "USART1", corrupt);
    sim_irq_at(9, "USART1", negative);
    sim_irq_at(11, "USART1", sync_lookalike);
    sim_run(20);
    EXPECT_EQ(g_bad_frames, 1);
    EXPECT_EQ(g_stats.frames, 3);
    EXPECT_EQ(g_stats.latest, 0x00A5); /* 16.5 °C: a byte that looks like SYNC is still data */
    EXPECT_EQ(g_stats.min, -40);
    EXPECT_EQ(g_stats.max, 0x00A5);
    EXPECT_EQ(g_bytes_dropped, 0);
}

static void first_frame(void) { send_frame(40); }
static void burst_of_three(void) {
    send_frame(41);
    send_frame(42);
    send_frame(43); /* 12 bytes in one go, while the parser is busy calibrating: the queue holds 8 */
}
static void one_more(void) { send_frame(44); }

TEST(a_burst_that_overflows_the_queue_drops_bytes_but_never_blocks_the_isr) {
    monitor_start();
    sim_irq_at(5, "USART1", first_frame);
    sim_irq_at(6, "USART1", burst_of_three);
    sim_irq_at(20, "USART1", one_more);
    sim_run(15);
    EXPECT_EQ(g_bytes_dropped, 4);            /* the third frame of the burst, counted */
    EXPECT_FALSE(USART1->SR & USART_SR_RXNE); /* dropped bytes are still acknowledged */
    EXPECT_EQ(g_stats.frames, 3);
    EXPECT_EQ(g_stats.latest, 42);
    sim_run(10);
    EXPECT_EQ(g_stats.frames, 4); /* still in step afterwards */
    EXPECT_EQ(g_stats.latest, 44);
    EXPECT_EQ(g_bad_frames, 0);
}

TEST(alarm_checks_run_every_100_ms_on_the_dot) {
    start();
    sim_irq_at(300, "USART1", one_more); /* the sensor resends a frame right on an alarm tick */
    sim_irq_at(700, "USART1", one_more);
    sim_run(1000);
    for (rtos_tick_t t = 0; t < 1000; t += ALARM_PERIOD) EXPECT_EQ(sim_first_run_at_or_after("alarm", t), t);
    EXPECT_EQ(sim_ran_ticks("alarm"), 20); /* 10 checks of 2 ms, no more */
    EXPECT_FALSE(alarm_on());
}

/* 5.0 °C, then too warm (9.5 °C) for frames 21..24, then 5.0 again, then -1.5 °C for 30..33. */
static int16_t excursions(unsigned n) {
    if (n >= 21 && n <= 24) return 95;
    if (n >= 30 && n <= 33) return -15;
    return 50;
}

TEST(alarm_follows_temperature_excursions_and_leaves_other_pins_alone) {
    GPIOB->ODR = 1u << 0; /* heartbeat LED on */
    temp_of = excursions;
    start();
    sim_run(550);
    EXPECT_FALSE(alarm_on());
    sim_run(100); /* the check at 600 sees frame 24 (9.5 °C) */
    EXPECT_TRUE(alarm_on());
    EXPECT_TRUE(GPIOB->ODR & 1u);
    sim_run(100); /* the check at 700 sees 5.0 °C again */
    EXPECT_FALSE(alarm_on());
    sim_run(100); /* the check at 800 sees frame 32 (-1.5 °C) */
    EXPECT_TRUE(alarm_on());
    EXPECT_EQ(g_stats.min, -15); /* signed: FF F1 */
    sim_run(100);
    EXPECT_FALSE(alarm_on());
    EXPECT_TRUE(GPIOB->ODR & 1u);
}

TEST(alarm_when_the_sensor_goes_silent) {
    frame_limit = 20; /* the last frame arrives at 485, then the cable falls out */
    start();
    sim_run(750);
    EXPECT_FALSE(alarm_on()); /* ~215 ms of silence is still fine */
    sim_run(100);
    EXPECT_TRUE(alarm_on()); /* > 300 ms at the check at 800 */
}

TEST(no_priority_inversion_on_the_data_lock) {
    start();
    sim_run(245);
    /* At 234 the EEPROM writer (1) takes the lock for 3 ms. At 235 a frame arrives and
     * the display (2) starts a 70 ms redraw. The parser needs the lock. */
    EXPECT_TRUE(g_stats.latest_tick >= 236 && g_stats.latest_tick <= 239); /* not ~300 */
    EXPECT_TRUE(sim_max_priority_seen("eeprom") > PRIO_DISPLAY);         /* it borrowed priority to finish */
    EXPECT_EQ(sim_max_priority_seen("display"), PRIO_DISPLAY);
    EXPECT_EQ(g_bytes_dropped, 0);
}

/* Rising through the first second (3.1 .. 7.0 °C), falling through the second (6.9 .. 3.0 °C). */
static int16_t ramp(unsigned n) { return (int16_t)(n <= 40 ? 30 + n : 110 - n); }

TEST(uplink_reports_each_window_without_holding_the_lock_while_sending) {
    temp_of = ramp;
    start();
    sim_run(1015);
    EXPECT_EQ(g_nreports, 1);
    EXPECT_EQ(g_stats.latest_tick, 1011); /* the frame at 1010 didn't wait for the modem */
    sim_run(1035);
    EXPECT_EQ(g_nreports, 2);
    EXPECT_TRUE(g_reports[0].tick >= 1000 && g_reports[0].tick <= 1005);
    EXPECT_TRUE(g_reports[1].tick >= 2000 && g_reports[1].tick <= 2005);
    EXPECT_EQ(g_reports[0].frames, 40);
    EXPECT_EQ(g_reports[0].min, 31);
    EXPECT_EQ(g_reports[0].max, 70);
    EXPECT_EQ(g_reports[1].frames, 80);
    EXPECT_EQ(g_reports[1].min, 30);
    EXPECT_EQ(g_reports[1].max, 69); /* a fresh window: the 7.0 °C peak belongs to the first */
    EXPECT_EQ(g_stats.latest_tick, 2036); /* nor did the one at 2035 */
    EXPECT_TRUE(sim_max_priority_seen("uplink") < PRIO_DISPLAY);
    EXPECT_STR_EQ(sim_running_at(2036), "display"); /* right after the parser: the modem doesn't delay the redraw */
}

TEST(three_seconds_of_service_without_deadlock_or_loss) {
    start();
    sim_run(3000);
    EXPECT_FALSE(sim_deadlocked());
    EXPECT_EQ(g_stats.frames, 120);
    EXPECT_EQ(g_bytes_dropped, 0);
    EXPECT_EQ(g_bad_frames, 0);
    EXPECT_EQ(g_nreports, 2); /* 1000 and 2000; the one at 3000 is due as the run ends */
    EXPECT_TRUE(g_eeprom_frames > 0);
    EXPECT_FALSE(alarm_on());
}
