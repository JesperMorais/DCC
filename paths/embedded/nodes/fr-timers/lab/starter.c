#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* Thermostat front panel: a heartbeat LED pattern and a backlight inactivity timeout. */

#define BLINK_STEP_MS 100
#define INACTIVITY_MS 1500
#define PATTERN_LEN 10

typedef struct {
    const uint8_t *pattern; /* 1 = on, one entry per BLINK_STEP_MS */
    uint8_t len;
    uint8_t step;
    bool on;
} led_t;

static const uint8_t k_heartbeat[PATTERN_LEN] = {1, 0, 1, 0, 0, 0, 0, 0, 0, 0}; /* double blink */

led_t g_status_led = {k_heartbeat, PATTERN_LEN, 0, false};
TimerHandle_t g_blink_timer;
TimerHandle_t g_idle_timer;
bool g_backlight_on;
uint32_t g_sleep_count;
TickType_t g_sleep_tick;

/* Runs in the timer service task: short, and never blocks. */
static void blink_cb(TimerHandle_t timer) {
    /* TODO: fetch the led_t from the timer ID, apply pattern[step], advance step */
    (void)timer;
}

static void idle_cb(TimerHandle_t timer) {
    /* TODO: backlight off, count the sleep, record the tick, stop blinking, LED off */
    (void)timer;
}

void panel_start(void) {
    /* TODO: create the blink (auto-reload, 100 ms, ID = &g_status_led) and the
     * idle (one-shot, 1500 ms) timers, turn the backlight on, start both. */
    (void)blink_cb;
    (void)idle_cb;
}

/* Called from the keypad task whenever a key is pressed. */
void user_activity(void) {
    /* TODO: wake up if asleep (restart the pattern from step 0), and push the
     * inactivity timeout back. */
}
