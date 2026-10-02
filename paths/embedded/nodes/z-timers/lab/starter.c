#include <zephyr/kernel.h>

#define SAMPLE_PERIOD_MS 100
#define BACKLIGHT_TIMEOUT_MS 500
#define I2C_READ_US 3000 /* reading the temperature sensor over I2C: 3 ms, and it sleeps on the bus */

uint32_t samples;
int64_t sample_at[64];
uint32_t missed; /* periods whose sample never happened */

bool backlight_on;
uint32_t backlight_offs;
int64_t backlight_off_at;

/* ---------------- periodic sampler ---------------- */
/* TODO: replace this thread with a periodic k_timer whose expiry submits a work item.
 * The work item takes the sample (and uses k_timer_status_get() to count missed periods). */
static void sampler_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    for (;;) {
        k_msleep(SAMPLE_PERIOD_MS);
        if (samples < 64) sample_at[samples] = k_uptime_get();
        samples++;
        k_busy_wait(I2C_READ_US);
    }
}
K_THREAD_DEFINE(sampler, 1024, sampler_thread, NULL, NULL, NULL, 4, 0, 0);

/* ---------------- backlight: one-shot ---------------- */
static void backlight_expiry(struct k_timer *timer) {
    (void)timer;
    backlight_on = false;
    backlight_offs++;
    backlight_off_at = k_uptime_get();
}
K_TIMER_DEFINE(backlight_timer, backlight_expiry, NULL);

void touch_isr(void) {
    backlight_on = true; /* TODO: and the timeout? */
}

/* ---------------- boot ---------------- */
static void app_main(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    backlight_on = true;
    k_timer_start(&backlight_timer, K_MSEC(BACKLIGHT_TIMEOUT_MS), K_NO_WAIT);
}
K_THREAD_DEFINE(app, 1024, app_main, NULL, NULL, NULL, 0, 0, 0);

/* ---- given: firmware-update writer. Cooperative, and it hogs the CPU for 220 ms ---- */
K_SEM_DEFINE(fw_evt, 0, 1);
void fw_isr(void) { k_sem_give(&fw_evt); }
static void fw_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    for (;;) {
        k_sem_take(&fw_evt, K_FOREVER);
        k_busy_wait(220000);
    }
}
K_THREAD_DEFINE(fwupd, 1024, fw_thread, NULL, NULL, NULL, -3, 0, 0);
