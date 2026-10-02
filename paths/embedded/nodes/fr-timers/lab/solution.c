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
    led_t *led = pvTimerGetTimerID(timer);
    led->on = led->pattern[led->step] != 0;
    led->step = (uint8_t)((led->step + 1) % led->len);
}

static void idle_cb(TimerHandle_t timer) {
    (void)timer;
    g_backlight_on = false;
    g_sleep_count++;
    g_sleep_tick = xTaskGetTickCount();
    xTimerStop(g_blink_timer, 0); /* block time 0: we're inside the daemon */
    g_status_led.on = false;
}

void panel_start(void) {
    g_blink_timer = xTimerCreate("blink", pdMS_TO_TICKS(BLINK_STEP_MS), pdTRUE, &g_status_led, blink_cb);
    g_idle_timer = xTimerCreate("idle", pdMS_TO_TICKS(INACTIVITY_MS), pdFALSE, NULL, idle_cb);
    configASSERT(g_blink_timer != NULL && g_idle_timer != NULL);
    g_backlight_on = true;
    xTimerStart(g_blink_timer, 0);
    xTimerStart(g_idle_timer, 0);
}

/* Called from the keypad task whenever a key is pressed. */
void user_activity(void) {
    if (!g_backlight_on) {
        g_backlight_on = true;
        g_status_led.step = 0; /* restart the heartbeat from the top */
        xTimerStart(g_blink_timer, 0);
    }
    xTimerReset(g_idle_timer, 0); /* push the timeout back */
}
