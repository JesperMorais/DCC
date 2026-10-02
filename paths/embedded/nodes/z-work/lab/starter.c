#include <zephyr/kernel.h>

#define HANDLE_US 2000 /* per press: debounce check, I2C write to the LED driver, BLE notify */

struct button {
    struct k_work work; /* embedded, so the handler can find its button */
    char name;
    volatile uint32_t presses; /* written only by the ISR */
    uint32_t handled;          /* written only by the work handler */
};

struct button btn_a = {.name = 'A'};
struct button btn_b = {.name = 'B'};

/* The record of what the app did, in order: "AAB..." */
char event_log[64];
int64_t event_at[64];
size_t n_events;

static void log_event(const struct button *b) {
    if (n_events < sizeof event_log - 1) {
        event_at[n_events] = k_uptime_get();
        event_log[n_events++] = b->name;
    }
}

static void button_work_handler(struct k_work *work) {
    (void)work;
    struct button *b = &btn_a; /* TODO: which button submitted this work? */
    /* TODO: one run may have to stand for several presses */
    log_event(b);
    k_busy_wait(HANDLE_US);
    b->handled++;
}

void button_a_isr(void) {
    btn_a.presses++;
    k_work_submit(&btn_a.work);
}

void button_b_isr(void) {
    /* TODO */
}

/* ---- given ---- */
static void app_main(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    k_work_init(&btn_a.work, button_work_handler);
    k_work_init(&btn_b.work, button_work_handler);
}

static void ui_thread(void *p1, void *p2, void *p3) {
    (void)p1; (void)p2; (void)p3;
    for (;;) k_busy_wait(10000); /* animating the display, all day long */
}

K_THREAD_DEFINE(app, 1024, app_main, NULL, NULL, NULL, 0, 0, 0);
K_THREAD_DEFINE(ui, 1024, ui_thread, NULL, NULL, NULL, 5, 0, 0);
