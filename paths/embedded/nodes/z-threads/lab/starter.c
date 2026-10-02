#include <zephyr/kernel.h>

/* Priorities, as the previous engineer (a FreeRTOS veteran) left them:
 * "Control is the most important thread, so it gets the biggest number.
 *  The radio is next. And the logger must never be interrupted mid-page,
 *  so I made it cooperative." */
#define CONTROL_PRIO 14
#define RADIO_PRIO   9
#define LOGGER_PRIO  K_PRIO_COOP(14) /* -2 */

#define CONTROL_PERIOD_MS 10
#define RADIO_PERIOD_MS 50

uint32_t control_runs;
uint32_t radio_bursts;
uint32_t flash_pages;

/* ---- TODO: fix the three priorities above. Don't change the thread bodies. ---- */

static void control_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    int64_t next = k_uptime_get();
    for (;;) {
        k_busy_wait(1000); /* read encoder, update PWM: 1 ms */
        control_runs++;
        next += CONTROL_PERIOD_MS;
        int64_t left = next - k_uptime_get();
        if (left > 0) k_msleep((int32_t)left);
    }
}

static void radio_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    for (;;) {
        k_busy_wait(1000); /* preamble */
        k_busy_wait(1000); /* payload  */
        k_busy_wait(1000); /* CRC      */
        radio_bursts++;
        k_msleep(RADIO_PERIOD_MS - 3);
    }
}

static void logger_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    for (;;) {
        k_busy_wait(25000); /* erase + program one flash page: 25 ms */
        flash_pages++;
        k_msleep(5);
    }
}

K_THREAD_DEFINE(control, 1024, control_thread, NULL, NULL, NULL, CONTROL_PRIO, 0, 0);
K_THREAD_DEFINE(radio, 1024, radio_thread, NULL, NULL, NULL, RADIO_PRIO, 0, 9);
K_THREAD_DEFINE(logger, 2048, logger_thread, NULL, NULL, NULL, LOGGER_PRIO, 0, 0);
