#include <zephyr/kernel.h>

/* ============================ the sensor hub ============================
 *
 *   imu_isr (every 4 ms) --msgq--> processing thread --mutex--> stats
 *                                                              ^
 *   report_timer (100 ms) --> report_work (sysworkq) ----------+
 *   logger thread (legacy, given) -----------------------------+
 *   ble thread (given): 30 ms radio bursts every 100 ms
 */

#define REPORT_PERIOD_MS 100
#define FILTER_US 1000 /* processing one IMU message: 1 ms */
#define REPORT_US 2000 /* formatting + UART for one report: 2 ms */
#define IMU_QUEUE_LEN 4

/* TODO: choose priorities. Zephyr: lower = more urgent, negative = cooperative. */
#define PROCESSING_PRIO 0
#define BLE_PRIO 0
#define LOGGER_PRIO 0

struct imu_msg {
    uint32_t seq;
    uint32_t t_ms;
    int16_t value;
};
K_MSGQ_DEFINE(imu_q, sizeof(struct imu_msg), IMU_QUEUE_LEN, 4);

/* ISR-owned counters. */
uint32_t isr_produced;
uint32_t isr_drops;

struct hub_stats {
    uint32_t processed;
    int32_t sum;
    uint32_t max_latency_ms; /* worst "processed at - captured at" */
};
struct hub_stats stats;
K_MUTEX_DEFINE(stats_lock);

struct report {
    int64_t t_ms;
    uint32_t processed;
    int32_t sum;
};
struct report reports[32];
uint32_t n_reports;

/* given: what the IMU measures for message number `seq` */
int16_t imu_value(uint32_t seq) { return (int16_t)((seq * 53u) % 400u) - 200; }

/* ---------------------------------------------------------------- ISR */
void imu_isr(void) {
    /* TODO: build an imu_msg (seq = ++isr_produced, t_ms = now, value = imu_value(seq))
     * and queue it without blocking. If it doesn't fit, count it in isr_drops. */
}

/* ---------------------------------------------------------------- processing */
static void processing_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    for (;;) {
        /* TODO: wait for a message, filter it (k_busy_wait(FILTER_US)), then update
         * stats under stats_lock: processed, sum, max_latency_ms. */
        k_msleep(1000);
    }
}

/* ---------------------------------------------------------------- reporter */
static void report_work_fn(struct k_work *work) {
    (void)work;
    /* TODO: note the time, take a consistent snapshot of stats under the lock,
     * then "send" it (k_busy_wait(REPORT_US)) and append it to reports[]. */
}
K_WORK_DEFINE(report_work, report_work_fn);

/* TODO: a periodic k_timer that gets report_work done every REPORT_PERIOD_MS. */

/* ---------------------------------------------------------------- boot */
static void hub_main(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    /* TODO: start the report timer */
}

/* ---------------------------------------------------------------- given, don't change */
uint32_t flash_writes;
static void logger_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    for (;;) {
        k_msleep(50);
        k_mutex_lock(&stats_lock, K_FOREVER);
        k_busy_wait(5000); /* legacy code: writes stats to flash while holding the lock */
        flash_writes++;
        k_mutex_unlock(&stats_lock);
    }
}

static void ble_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    for (;;) {
        k_busy_wait(30000); /* connection event + advertising */
        k_msleep(70);
    }
}

K_THREAD_DEFINE(hub, 1024, hub_main, NULL, NULL, NULL, 0, 0, 0);
K_THREAD_DEFINE(processing, 2048, processing_thread, NULL, NULL, NULL, PROCESSING_PRIO, 0, 0);
K_THREAD_DEFINE(ble, 2048, ble_thread, NULL, NULL, NULL, BLE_PRIO, 0, 51);
K_THREAD_DEFINE(logger, 1024, logger_thread, NULL, NULL, NULL, LOGGER_PRIO, 0, 0);
