#include <zephyr/kernel.h>

/* Zephyr priorities: LOWER number = MORE urgent. Negative = cooperative.
 *
 *  radio   : cooperative, so its 3-step TX burst can never be interleaved.
 *            Coop is fine here because the burst is short and bounded (3 ms).
 *  control : the most urgent *preemptible* thread, so it takes the CPU the
 *            moment it wakes (unless a coop thread is mid-burst).
 *  logger  : long, boring flash writes. Preemptible and least urgent:
 *            it soaks up whatever CPU is left. */
#define RADIO_PRIO   K_PRIO_COOP(14) /* -2 */
#define CONTROL_PRIO 2
#define LOGGER_PRIO  10

#define CONTROL_PERIOD_MS 10
#define RADIO_PERIOD_MS 50
#define RADIO_FIRST_MS 9

uint32_t control_runs;
uint32_t radio_bursts;
uint32_t flash_pages;

/* ---- do not change the thread bodies below; only the priorities ---- */

static void control_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    int64_t next = k_uptime_get();
    for (;;) {
        k_busy_wait(1000); /* read encoder, update PWM: 1 ms */
        control_runs++;
        next += CONTROL_PERIOD_MS;
        k_sleep(K_TIMEOUT_ABS_MS(next)); /* until the next release: no drift */
    }
}

static void radio_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    int64_t next = RADIO_FIRST_MS;
    for (;;) {
        k_sleep(K_TIMEOUT_ABS_MS(next)); /* the radio's slot: 9, 59, 109, ... */
        k_busy_wait(1000); /* preamble */
        k_busy_wait(1000); /* payload  */
        k_busy_wait(1000); /* CRC      */
        radio_bursts++;
        next += RADIO_PERIOD_MS;
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
K_THREAD_DEFINE(radio, 1024, radio_thread, NULL, NULL, NULL, RADIO_PRIO, 0, 0);
K_THREAD_DEFINE(logger, 2048, logger_thread, NULL, NULL, NULL, LOGGER_PRIO, 0, 0);
