#include "FreeRTOS.h"
#include "sim.h"
#include "task.h"
#include "timers.h"

/* The keypad: presses a key at each tick in `presses` (ascending, 0-terminated). */
static TickType_t presses[8];
static void keypad_task(void *pvParameters) {
    (void)pvParameters;
    TickType_t wake = 0;
    for (int i = 0; presses[i]; i++) {
        vTaskDelayUntil(&wake, presses[i] - wake);
        sim_mark("key press");
        user_activity();
    }
    vTaskDelete(NULL);
}

static void start_with_presses(TickType_t a, TickType_t b) {
    presses[0] = a;
    presses[1] = b;
    panel_start();
    xTaskCreate(keypad_task, "keypad", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
}

/* LED state after everything that happened at tick t has run. */
static bool led[4000];
static void sample(TickType_t from, TickType_t to) {
    for (TickType_t t = from; t < to; t++) {
        sim_run(1);
        led[t] = g_status_led.on;
    }
}

static bool on_between(TickType_t t) {
    return (t >= 100 && t < 200) || (t >= 300 && t < 400) || (t >= 1100 && t < 1200) || (t >= 1300 && t < 1400);
}

TEST(heartbeat_pattern_is_exact) {
    start_with_presses(0, 0);
    sample(0, 1450);
    for (TickType_t t = 0; t < 1450; t++) EXPECT_EQ(led[t], on_between(t));
}

TEST(blink_timer_carries_its_led_as_the_timer_id) {
    panel_start();
    EXPECT_NOT_NULL(g_blink_timer);
    EXPECT_NOT_NULL(g_idle_timer);
    EXPECT_PTR_EQ(pvTimerGetTimerID(g_blink_timer), &g_status_led);
}

TEST(backlight_sleeps_1500ms_after_start_and_the_led_stops) {
    start_with_presses(0, 0);
    sim_run(1600);
    EXPECT_FALSE(g_backlight_on);
    EXPECT_EQ(g_sleep_tick, 1500);
    EXPECT_FALSE(g_status_led.on);
    uint8_t step = g_status_led.step;
    sim_run(1000);
    EXPECT_EQ(g_status_led.step, step); /* blink timer was stopped */
    EXPECT_FALSE(g_status_led.on);
}

TEST(each_key_press_pushes_the_timeout_back) {
    start_with_presses(400, 600);
    sim_run(2050);
    EXPECT_TRUE(g_backlight_on); /* would have slept at 1500 */
    sim_run(100);
    EXPECT_FALSE(g_backlight_on);
    EXPECT_EQ(g_sleep_tick, 2100); /* exactly 1500 ms after the last press */
}

TEST(the_inactivity_timer_is_one_shot) {
    start_with_presses(0, 0);
    sim_run(5000);
    EXPECT_EQ(g_sleep_count, 1);
}

TEST(a_key_press_wakes_the_panel_and_restarts_the_pattern) {
    start_with_presses(2000, 0);
    sample(0, 3600);
    EXPECT_FALSE(led[2099]);
    EXPECT_TRUE(led[2100]); /* step 0 again, 100 ms after the wake */
    EXPECT_FALSE(led[2200]);
    EXPECT_TRUE(led[2300]);
    EXPECT_EQ(g_sleep_count, 2);
    EXPECT_EQ(g_sleep_tick, 3500);
}
