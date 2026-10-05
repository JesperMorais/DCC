/* watchdog.c: a software timer that checks the sensor task is alive. */
#include "app.h"
#include "timers.h"

static volatile uint32_t checkins;
static bool tripped;

void watchdog_checkin(void)
{
    taskENTER_CRITICAL();
    checkins++;
    taskEXIT_CRITICAL();
}

/* Runs in the timer daemon: it must never block. */
static void watchdog_cb(TimerHandle_t t)
{
    (void)t;
    taskENTER_CRITICAL();
    uint32_t n = checkins;
    checkins = 0;
    taskEXIT_CRITICAL();

    if (n > 0) {
        log_now("HEARTBEAT checkins=%lu", (unsigned long)n);
    } else if (!tripped) {
        tripped = true;
        log_now("WATCHDOG sensor stalled");
        xTaskNotify(g_safety_task, SHUTDOWN_WATCHDOG, eSetBits);
    }
}

void watchdog_start(void)
{
    TimerHandle_t t = xTimerCreate("wdog", pdMS_TO_TICKS(WATCHDOG_PERIOD_MS), pdTRUE, NULL, watchdog_cb);
    configASSERT(t != NULL);
    xTimerStart(t, 0);
}
