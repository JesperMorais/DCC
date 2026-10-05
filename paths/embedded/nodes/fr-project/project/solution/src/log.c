/* log.c: the logger task. On the POSIX port, stdio must only be used from
 * one task: printf takes a pthread lock, and if the kernel switched away from
 * a task holding it, the next task to print would hang. So every other task
 * sends its line here, by value, through a queue. */
#include <stdarg.h>
#include <stdio.h>

#include "app.h"

static QueueHandle_t log_queue;

void log_init(void)
{
    log_queue = xQueueCreate(LOG_QUEUE_LEN, sizeof(log_line_t));
    configASSERT(log_queue != NULL);
}

static bool vlog(TickType_t tick, bool last, TickType_t wait, const char *fmt, va_list ap)
{
    log_line_t line = { .tick = tick, .last = last };
    vsnprintf(line.text, sizeof line.text, fmt, ap);
    return xQueueSend(log_queue, &line, wait) == pdPASS;
}

/* Never blocks, so it's safe from the timer daemon. A full queue drops the line. */
bool log_at(TickType_t tick, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    bool ok = vlog(tick, false, 0, fmt, ap);
    va_end(ap);
    return ok;
}

bool log_now(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    bool ok = vlog(xTaskGetTickCount(), false, 0, fmt, ap);
    va_end(ap);
    return ok;
}

/* The final line waits for room: it must not be lost. */
void log_last(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vlog(xTaskGetTickCount(), true, portMAX_DELAY, fmt, ap);
    va_end(ap);
}

void log_task(void *arg)
{
    (void)arg;
    log_line_t line;
    for (;;) {
        xQueueReceive(log_queue, &line, portMAX_DELAY);
        printf("%lu %s\n", (unsigned long)line.tick, line.text);
        if (line.last) {
            fflush(stdout);
            vTaskEndScheduler(); /* main() returns from vTaskStartScheduler() */
        }
    }
}
