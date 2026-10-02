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

/* ---------------- periodic sampler: timer -> work ---------------- */
static void sample_expiry(struct k_timer *timer);
K_TIMER_DEFINE(sample_timer, sample_expiry, NULL);

static void sample_work_fn(struct k_work *work) {
    (void)work;
    /* How many times did the timer expire since we last looked? Normally 1.
     * If the workqueue was stalled, several expiries collapsed into one run. */
    uint32_t expiries = k_timer_status_get(&sample_timer);
    if (expiries > 1) missed += expiries - 1;

    if (samples < 64) sample_at[samples] = k_uptime_get();
    samples++;
    k_busy_wait(I2C_READ_US);
}
K_WORK_DEFINE(sample_work, sample_work_fn);

/* ISR context: no sleeping, no I2C. Just hand the job to a thread. */
static void sample_expiry(struct k_timer *timer) {
    (void)timer;
    k_work_submit(&sample_work);
}

/* ---------------- backlight: one-shot, restarted on activity ---------------- */
static void backlight_expiry(struct k_timer *timer) {
    (void)timer;
    backlight_on = false; /* a GPIO write: fine in ISR context */
    backlight_offs++;
    backlight_off_at = k_uptime_get();
}
K_TIMER_DEFINE(backlight_timer, backlight_expiry, NULL);

static void backlight_kick(void) {
    backlight_on = true;
    /* Real Zephyr restarts a running timer on k_timer_start() alone;
     * the simulator currently needs the explicit stop (see the prompt). */
    k_timer_stop(&backlight_timer);
    k_timer_start(&backlight_timer, K_MSEC(BACKLIGHT_TIMEOUT_MS), K_NO_WAIT); /* one-shot */
}

void touch_isr(void) {
    backlight_kick();
}

/* ---------------- boot ---------------- */
static void app_main(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    backlight_kick();
    k_timer_start(&sample_timer, K_MSEC(SAMPLE_PERIOD_MS), K_MSEC(SAMPLE_PERIOD_MS));
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
