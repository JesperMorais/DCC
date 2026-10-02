#include <zephyr/kernel.h>

struct imu_sample {
    uint32_t seq;  /* 1, 2, 3, ... */
    uint32_t t_ms; /* when the ISR captured it */
    int16_t x;     /* acceleration, milli-g */
};

#define IMU_QUEUE_LEN 8
#define STALE_MS 8 /* the fusion filter can't use samples older than this */
K_MSGQ_DEFINE(imu_q, sizeof(struct imu_sample), IMU_QUEUE_LEN, 4);

/* Written only by the ISR. */
uint32_t produced; /* samples the ISR captured */
uint32_t dropped;  /* samples thrown away because the queue was full */

/* Written only by the fusion thread. */
struct imu_stats {
    uint32_t consumed;
    int32_t sum_x;
    uint32_t last_seq;
    uint32_t stale; /* samples that were older than STALE_MS when consumed */
} stats;

/* given: what the accelerometer returns for sample number `seq` */
int16_t sample_value(uint32_t seq) { return (int16_t)((seq * 37u) % 200u) - 100; }

/* given: the BLE controller grabs the CPU for 20 ms when this IRQ fires */
K_SEM_DEFINE(radio_evt, 0, 1);
void radio_isr(void) { k_sem_give(&radio_evt); }

/* Accelerometer "data ready", 1 kHz. */
void imu_isr(void) {
    /* TODO: build a struct imu_sample (seq = ++produced, t_ms, x = sample_value(seq))
     * and put it in imu_q without ever blocking. When the queue is full, drop the
     * OLDEST sample (and count it in `dropped`) so the newest one gets in. */
}

static void fusion_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    for (;;) {
        /* TODO: take samples out of imu_q and update `stats`:
         * consumed, sum_x, last_seq, and stale (now - t_ms > STALE_MS). */
        k_msleep(1000);
    }
}

static void radio_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    for (;;) {
        k_sem_take(&radio_evt, K_FOREVER);
        k_busy_wait(20000); /* a long BLE connection event */
    }
}

K_THREAD_DEFINE(fusion, 1024, fusion_thread, NULL, NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(radio, 1024, radio_thread, NULL, NULL, NULL, 1, 0, 0);
