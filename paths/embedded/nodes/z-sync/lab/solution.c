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

/* A counting semaphore: cards can arrive faster than we process them,
 * and every one of them must be counted. */
K_SEM_DEFINE(card_sem, 0, 16);

/* A real mutex: owned, recursive, with priority inheritance. */
K_MUTEX_DEFINE(log_lock);

/* Card reader "data ready" interrupt. ISR rule: never block. Just signal. */
void card_isr(void) {
    k_sem_give(&card_sem);
}

/* Called with or without log_lock held: Zephyr mutexes are recursive. */
static void log_touch(void) {
    k_mutex_lock(&log_lock, K_FOREVER);
    door_log.updates++;
    door_log.last_change_ms = k_uptime_get();
    k_mutex_unlock(&log_lock);
}

static void log_card(void) {
    k_mutex_lock(&log_lock, K_FOREVER);
    door_log.cards++;
    log_touch(); /* re-locks log_lock: fine, we already own it */
    k_mutex_unlock(&log_lock);
}

static void log_timeout(void) {
    k_mutex_lock(&log_lock, K_FOREVER);
    door_log.timeouts++;
    log_touch();
    k_mutex_unlock(&log_lock);
}

static void reader_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    for (;;) {
        int rc = k_sem_take(&card_sem, K_MSEC(READER_TIMEOUT_MS));
        if (rc == -EAGAIN) {
            log_timeout(); /* no card arrived: not a card! */
            continue;
        }
        k_busy_wait(CARD_WORK_US);
        log_card();
    }
}

/* ---- given: uploads the log to the building server every 50 ms ---- */
uint32_t uploaded_cards;
static void cloud_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    int64_t next = 0;
    for (;;) {
        next += 50;
        k_sleep(K_TIMEOUT_ABS_MS(next)); /* uploads at 50, 100, 150, ... */
        k_mutex_lock(&log_lock, K_FOREVER);
        k_busy_wait(5000); /* slow SPI modem: the log is locked for 5 ms */
        uploaded_cards = door_log.cards;
        k_mutex_unlock(&log_lock);
    }
}

/* ---- given: redraws the e-ink display, 20 ms of CPU every 100 ms ---- */
static void ui_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    int64_t next = 52;
    for (;;) {
        k_sleep(K_TIMEOUT_ABS_MS(next)); /* redraws at 52, 152, 252, ... */
        k_busy_wait(20000);
        next += 100;
    }
}

K_THREAD_DEFINE(reader, 1024, reader_thread, NULL, NULL, NULL, 2, 0, 0);
K_THREAD_DEFINE(ui, 1024, ui_thread, NULL, NULL, NULL, 6, 0, 0);
K_THREAD_DEFINE(cloud, 1024, cloud_thread, NULL, NULL, NULL, 10, 0, 0);
