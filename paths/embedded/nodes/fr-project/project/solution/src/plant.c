/* plant.c: sensor, control and safety tasks, and the e-stop ISR. */
#include "app.h"
#include "hw.h"

static volatile uint32_t estop_irqs;
static volatile TickType_t estop_first_irq_tick;

void sensor_task(void *arg)
{
    (void)arg;
    TickType_t wake = xTaskGetTickCount();
    for (;;) {
        /* Absolute deadlines: the 2-6 ms conversion doesn't push the next one. */
        xTaskDelayUntil(&wake, pdMS_TO_TICKS(SENSOR_PERIOD_MS));
        reading_t r = { .tick = wake };
        r.level_mm = hw_read_level();
        watchdog_checkin();
        log_at(r.tick, "SENSOR level=%d", r.level_mm);
        if (xQueueSend(g_readings, &r, 0) != pdPASS) log_now("SENSOR queue full, sample dropped");
    }
}

void control_task(void *arg)
{
    (void)arg;
    reading_t r;
    for (;;) {
        xQueueReceive(g_readings, &r, portMAX_DELAY);

        /* Decide and act under the lock: otherwise the safety task could latch
         * a shutdown between our check and hw_pump_set(true). */
        xSemaphoreTake(g_status_mutex, portMAX_DELAY);
        plant_status_t *s = &g_status;
        s->samples++;
        s->last_level_mm = r.level_mm;
        if (r.level_mm > s->max_level_mm) s->max_level_mm = r.level_mm;
        bool want = s->pump_on;
        if (s->estop || s->fault) want = false;
        else if (r.level_mm >= LEVEL_HIGH_MM) want = true;
        else if (r.level_mm <= LEVEL_LOW_MM) want = false;
        bool changed = want != s->pump_on;
        if (changed) {
            s->pump_on = want;
            if (want) s->pump_starts++;
            hw_pump_set(want);
        }
        xSemaphoreGive(g_status_mutex);

        if (changed) log_now("PUMP %s level=%d", want ? "on" : "off", r.level_mm);
    }
}

void estop_isr(void)
{
    BaseType_t woken = pdFALSE;
    if (estop_irqs++ == 0) estop_first_irq_tick = xTaskGetTickCountFromISR();
    xTaskNotifyFromISR(g_safety_task, SHUTDOWN_ESTOP, eSetBits, &woken);
    portYIELD_FROM_ISR(woken); /* run the safety task now, not at the next tick */
}

uint32_t estop_irq_count(void) { return estop_irqs; }

void safety_task(void *arg)
{
    (void)arg;
    uint32_t reasons;
    for (;;) {
        xTaskNotifyWait(0, UINT32_MAX, &reasons, portMAX_DELAY);

        xSemaphoreTake(g_status_mutex, portMAX_DELAY);
        bool first_estop = (reasons & SHUTDOWN_ESTOP) && !g_status.estop;
        if (reasons & SHUTDOWN_ESTOP) g_status.estop = true;
        if (reasons & SHUTDOWN_WATCHDOG) g_status.fault = true;
        g_status.pump_on = false;
        hw_pump_set(false);
        xSemaphoreGive(g_status_mutex);

        if (first_estop) log_now("ESTOP irq=%lu", (unsigned long)estop_first_irq_tick);
    }
}
