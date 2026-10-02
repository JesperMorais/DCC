/* FreeRTOS API facade over the daily.ts simulator. */
#define _GNU_SOURCE

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

/* FreeRTOS semantics: a task woken from an ISR only runs before the next tick if the ISR yields. */
static void frt_isr_mode(void) {
    static bool done;
    if (!done) {
        done = true;
        sim_set_isr_auto_yield(false);
    }
}

static void check_prio(UBaseType_t p) {
    if (p >= configMAX_PRIORITIES) sim_fail("priority %lu is out of range: valid priorities are 0..%d (configMAX_PRIORITIES - 1)", p, configMAX_PRIORITIES - 1);
}

BaseType_t xTaskCreate(TaskFunction_t fn, const char *name, uint32_t stack_depth, void *param, UBaseType_t priority, TaskHandle_t *created) {
    frt_isr_mode();
    check_prio(priority);
    if (stack_depth < 64) sim_fail("stack depth %u words is too small for a task (try configMINIMAL_STACK_SIZE or more)", (unsigned)stack_depth);
    TaskHandle_t t = rtos_task_create(name, fn, param, (int)priority);
    if (created) *created = t;
    return pdPASS;
}

void vTaskDelete(TaskHandle_t task) {
    if (task == NULL || task == rtos_self()) rtos_task_exit();
    sim_fail("vTaskDelete() of another task isn't supported in the simulator — let the task delete itself");
}

void vTaskDelay(TickType_t ticks) { rtos_delay(ticks); }
void vTaskDelayUntil(TickType_t *prev, TickType_t period) { rtos_delay_until(prev, period); }
BaseType_t xTaskDelayUntil(TickType_t *prev, TickType_t period) {
    TickType_t before = rtos_now();
    TickType_t target = *prev + period;
    rtos_delay_until(prev, period);
    return target > before ? pdTRUE : pdFALSE;
}
TickType_t xTaskGetTickCount(void) { return rtos_now(); }
TickType_t xTaskGetTickCountFromISR(void) { return rtos_now(); }
TaskHandle_t xTaskGetCurrentTaskHandle(void) { return rtos_self(); }
UBaseType_t uxTaskPriorityGet(TaskHandle_t t) { return (UBaseType_t)rtos_task_priority(t ? t : rtos_self()); }
void vTaskPrioritySet(TaskHandle_t t, UBaseType_t p) {
    check_prio(p);
    rtos_task_set_priority(t ? t : rtos_self(), (int)p);
}
const char *pcTaskGetName(TaskHandle_t t) { return rtos_task_name(t ? t : rtos_self()); }

BaseType_t xTaskNotifyGive(TaskHandle_t t) {
    rtos_notify_give(t);
    return pdPASS;
}
void vTaskNotifyGiveFromISR(TaskHandle_t t, BaseType_t *woken) {
    frt_isr_mode();
    rtos_notify_give(t);
    if (woken) *woken = pdTRUE;
}
uint32_t ulTaskNotifyTake(BaseType_t clear, TickType_t timeout) { return rtos_notify_take(clear != pdFALSE, timeout); }

/* ---- queues ---- */
struct dts_frt_queue { rtos_queue_t *q; };

QueueHandle_t xQueueCreate(UBaseType_t length, UBaseType_t item_size) {
    frt_isr_mode();
    QueueHandle_t h = sim_calloc(sizeof *h);
    if (!h) return NULL;
    h->q = rtos_queue_create(length, item_size);
    return h;
}
void vQueueDelete(QueueHandle_t q) { (void)q; /* kernel-owned; freed at the end of the run */ }
BaseType_t xQueueSend(QueueHandle_t q, const void *item, TickType_t timeout) {
    configASSERT(q != NULL);
    if (sim_in_isr()) sim_fail("xQueueSend() called from an ISR — use xQueueSendFromISR()");
    return rtos_queue_send(q->q, item, timeout) ? pdPASS : errQUEUE_FULL;
}
BaseType_t xQueueSendToBack(QueueHandle_t q, const void *item, TickType_t timeout) { return xQueueSend(q, item, timeout); }
BaseType_t xQueueReceive(QueueHandle_t q, void *out, TickType_t timeout) {
    configASSERT(q != NULL);
    if (sim_in_isr()) sim_fail("xQueueReceive() called from an ISR — that would block");
    return rtos_queue_receive(q->q, out, timeout) ? pdPASS : errQUEUE_EMPTY;
}
BaseType_t xQueueSendFromISR(QueueHandle_t q, const void *item, BaseType_t *woken) {
    configASSERT(q != NULL);
    if (!sim_in_isr()) sim_fail("xQueueSendFromISR() called outside an ISR — use xQueueSend() in tasks");
    bool ok = rtos_queue_send(q->q, item, RTOS_NO_WAIT);
    if (ok && woken) *woken = pdTRUE;
    return ok ? pdPASS : errQUEUE_FULL;
}
BaseType_t xQueueSendToBackFromISR(QueueHandle_t q, const void *item, BaseType_t *woken) { return xQueueSendFromISR(q, item, woken); }
UBaseType_t uxQueueMessagesWaiting(QueueHandle_t q) { return q ? (UBaseType_t)rtos_queue_count(q->q) : 0; }

/* ---- semaphores & mutexes ---- */
struct dts_frt_sem { rtos_sem_t *sem; rtos_mutex_t *mutex; };

static SemaphoreHandle_t new_sem(void) {
    frt_isr_mode();
    SemaphoreHandle_t s = sim_calloc(sizeof *s);
    return s;
}
SemaphoreHandle_t xSemaphoreCreateBinary(void) {
    SemaphoreHandle_t s = new_sem();
    if (s) s->sem = rtos_sem_create(0, 1);
    return s;
}
SemaphoreHandle_t xSemaphoreCreateCounting(UBaseType_t max, UBaseType_t initial) {
    SemaphoreHandle_t s = new_sem();
    if (s) s->sem = rtos_sem_create((unsigned)initial, (unsigned)max);
    return s;
}
SemaphoreHandle_t xSemaphoreCreateMutex(void) {
    SemaphoreHandle_t s = new_sem();
    if (s) s->mutex = rtos_mutex_create(true);
    return s;
}
void vSemaphoreDelete(SemaphoreHandle_t s) { (void)s; }
BaseType_t xSemaphoreTake(SemaphoreHandle_t s, TickType_t timeout) {
    configASSERT(s != NULL);
    if (s->mutex) return rtos_mutex_lock(s->mutex, timeout) ? pdTRUE : pdFALSE;
    if (sim_in_isr() && timeout != 0) sim_fail("xSemaphoreTake() with a timeout in an ISR would block");
    return rtos_sem_take(s->sem, timeout) ? pdTRUE : pdFALSE;
}
BaseType_t xSemaphoreGive(SemaphoreHandle_t s) {
    configASSERT(s != NULL);
    if (sim_in_isr()) sim_fail("xSemaphoreGive() called from an ISR — use xSemaphoreGiveFromISR()");
    if (s->mutex) {
        if (rtos_mutex_owner(s->mutex) != rtos_self()) return pdFALSE; /* real FreeRTOS: give by non-holder fails */
        rtos_mutex_unlock(s->mutex);
        return pdTRUE;
    }
    return rtos_sem_give(s->sem) ? pdTRUE : pdFALSE;
}
BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t s, BaseType_t *woken) {
    configASSERT(s != NULL);
    if (s->mutex) sim_fail("mutexes can't be given from an ISR");
    if (!sim_in_isr()) sim_fail("xSemaphoreGiveFromISR() called outside an ISR — use xSemaphoreGive() in tasks");
    bool ok = rtos_sem_give(s->sem);
    if (ok && woken) *woken = pdTRUE;
    return ok ? pdTRUE : pdFALSE;
}
UBaseType_t uxSemaphoreGetCount(SemaphoreHandle_t s) { return s && s->sem ? rtos_sem_count(s->sem) : 0; }

/* ---- software timers: expiry → command to the daemon task, which runs the callback ---- */
struct dts_frt_timer { rtos_timer_t *t; TimerCallbackFunction_t cb; void *id; TickType_t period; };
static rtos_queue_t *daemon_q;

static void daemon_task(void *arg) {
    (void)arg;
    for (;;) {
        TimerHandle_t h;
        rtos_queue_receive(daemon_q, &h, RTOS_WAIT_FOREVER);
        h->cb(h);
    }
}
static void on_expiry(void *arg) {
    TimerHandle_t h = arg;
    rtos_queue_send(daemon_q, &h, RTOS_NO_WAIT);
    sim_isr_yield(); /* the tick ISR yields to the daemon if it's higher priority */
}
static void ensure_daemon(void) {
    if (daemon_q) return;
    daemon_q = rtos_queue_create(16, sizeof(TimerHandle_t));
    rtos_task_create("Tmr Svc", daemon_task, NULL, configTIMER_TASK_PRIORITY);
}

TimerHandle_t xTimerCreate(const char *name, TickType_t period, UBaseType_t auto_reload, void *id, TimerCallbackFunction_t cb) {
    frt_isr_mode();
    configASSERT(period > 0);
    TimerHandle_t h = sim_calloc(sizeof *h);
    if (!h) return NULL;
    h->cb = cb;
    h->id = id;
    h->period = period;
    h->t = rtos_timer_create(name, period, auto_reload != pdFALSE, on_expiry, h);
    return h;
}
BaseType_t xTimerStart(TimerHandle_t t, TickType_t wait) {
    (void)wait;
    configASSERT(t != NULL);
    ensure_daemon();
    rtos_timer_start(t->t, t->period);
    return pdPASS;
}
BaseType_t xTimerStop(TimerHandle_t t, TickType_t wait) {
    (void)wait;
    configASSERT(t != NULL);
    rtos_timer_stop(t->t);
    return pdPASS;
}
BaseType_t xTimerReset(TimerHandle_t t, TickType_t wait) { return xTimerStart(t, wait); }
void *pvTimerGetTimerID(TimerHandle_t t) { return t ? t->id : NULL; }
