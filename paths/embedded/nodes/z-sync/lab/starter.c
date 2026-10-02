#include <zephyr/kernel.h>

#define READER_TIMEOUT_MS 100 /* no card for 100 ms: log a heartbeat timeout */
#define CARD_WORK_US 2000     /* decrypting a card's credentials: 2 ms */

struct door_log {
    uint32_t cards;    /* cards processed */
    uint32_t timeouts; /* 100 ms windows with no card */
    uint32_t updates;  /* bumped by log_touch() on every change */
    int64_t last_change_ms;
};
struct door_log door_log;

K_SEM_DEFINE(card_sem, 0, 1);

/* "A semaphore that starts at 1 is a lock, right?" */
K_SEM_DEFINE(log_lock, 1, 1);

static volatile bool card_ready;

/* Card reader "data ready" interrupt. */
void card_isr(void) {
    card_ready = true; /* TODO: signal the reader properly */
}

static void log_touch(void) {
    k_sem_take(&log_lock, K_FOREVER);
    door_log.updates++;
    door_log.last_change_ms = k_uptime_get();
    k_sem_give(&log_lock);
}

static void log_card(void) {
    k_sem_take(&log_lock, K_FOREVER);
    door_log.cards++;
    log_touch();
    k_sem_give(&log_lock);
}

static void log_timeout(void) {
    k_sem_take(&log_lock, K_FOREVER);
    door_log.timeouts++;
    log_touch();
    k_sem_give(&log_lock);
}

static void reader_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    for (;;) {
        /* TODO: block on card_sem with a 100 ms timeout instead of polling,
         * and tell a timeout (-EAGAIN) apart from a real card (0). */
        k_msleep(10);
        if (card_ready) {
            card_ready = false;
            k_busy_wait(CARD_WORK_US);
            log_card();
        }
    }
}

/* ---- given: uploads the log to the building server every 50 ms ---- */
uint32_t uploaded_cards;
static void cloud_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    for (;;) {
        k_msleep(50);
        k_sem_take(&log_lock, K_FOREVER);
        k_busy_wait(5000); /* slow SPI modem: the log is locked for 5 ms */
        uploaded_cards = door_log.cards;
        k_sem_give(&log_lock);
    }
}

/* ---- given: redraws the e-ink display, 20 ms of CPU every 100 ms ---- */
static void ui_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    for (;;) {
        k_busy_wait(20000);
        k_msleep(80);
    }
}

K_THREAD_DEFINE(reader, 1024, reader_thread, NULL, NULL, NULL, 2, 0, 0);
K_THREAD_DEFINE(ui, 1024, ui_thread, NULL, NULL, NULL, 6, 0, 52);
K_THREAD_DEFINE(cloud, 1024, cloud_thread, NULL, NULL, NULL, 10, 0, 0);
