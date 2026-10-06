/* hw.c: the simulated board. Given: don't edit (the tests rely on it).
 *
 * How the "hardware" works on the FreeRTOS POSIX port
 * ---------------------------------------------------
 * The POSIX port runs every FreeRTOS task on its own pthread, and lets only
 * one of them run at a time. Its only real interrupt is the tick: a helper
 * thread sends SIGALRM to the thread of the running task every millisecond,
 * and the signal handler is the tick ISR. "Interrupts disabled" means
 * "signals blocked on this thread"; taskENTER_CRITICAL() does exactly that.
 *
 * This file hooks into the tick ISR (vApplicationTickHook) to advance the
 * physics one millisecond at a time. The e-stop interrupt is a second signal,
 * SIGUSR2, raised from the tick hook. Every thread except the running task's
 * has all signals blocked, so the kernel delivers it to the running task's
 * thread as soon as that thread unblocks signals, i.e. right after the tick
 * ISR returns, or when the running task leaves a critical section. That is how
 * a pending IRQ behaves on a microcontroller, too. Its handler calls your ISR,
 * on whichever task's thread was running: interrupt context, where only
 * FromISR APIs are allowed (calling hw_pump_set() or hw_read_level() from it
 * aborts with a message). portYIELD_FROM_ISR() switches to the woken task
 * before the handler returns, the same way the port's own tick ISR does.
 *
 * The same goes for the kernel's task-only API. The Makefile links the plant
 * with -Wl,--wrap=<function> for each one listed at the end of this file, so
 * a call such as xTaskNotify() reaches __wrap_xTaskGenericNotify() here first.
 * In a task it goes straight on to the real kernel function; in the ISR it
 * aborts and names the FromISR API to use instead.
 */
#include "hw.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "timers.h"

#define LEVEL_START     400
#define LEVEL_MAX       1000
#define INFLOW_PER_MS   1
#define PUMP_PER_MS     2

static int run_ms = 3000;
static long estop_at = -1;
static long stall_at = -1;

/* Board state. The tick hook runs with signals blocked; task-side functions
 * use a critical section, so the two never interleave. */
static unsigned long hw_ticks;
static int level = LEVEL_START;
static volatile bool pump_on;
static volatile bool estop_pressed;
static void (*volatile estop_isr)(void);

static int max_level = LEVEL_START;
static unsigned long overflow_ticks, dry_run_ticks, pump_starts;
static unsigned long estop_irqs, estop_pump_ticks;
static int estop_preempted = -1; /* see estop_signal_handler() */
static __thread int in_isr;       /* per thread: true while this thread runs an ISR */
static unsigned long adc_reads;
static long adc_last_start = -1, adc_period_min = -1, adc_period_max = -1;

static void usage(const char *prog, const char *bad)
{
    fprintf(stderr, "%s: unknown or incomplete option \"%s\"\n", prog, bad);
    fprintf(stderr, "usage: %s [--run-ms N] [--estop-at T] [--sensor-stall-at T]\n", prog);
    exit(2);
}

static void report(void)
{
    fprintf(stderr,
            "hw: ticks=%lu max_level=%d overflow_ticks=%lu dry_run_ticks=%lu pump_starts=%lu "
            "pump=%s estop_irqs=%lu estop_pump_ticks=%lu estop_preempted=%d adc_reads=%lu adc_period_min=%ld "
            "adc_period_max=%ld\n",
            hw_ticks, max_level, overflow_ticks, dry_run_ticks, pump_starts, pump_on ? "on" : "off",
            estop_irqs, estop_pump_ticks, estop_preempted, adc_reads, adc_period_min, adc_period_max);
}

static void must_not_be_in_isr(const char *what)
{
    if (in_isr) {
        fprintf(stderr, "hw: %s called from an ISR: only FromISR APIs are allowed there\n", what);
        abort();
    }
}

static void estop_signal_handler(int sig)
{
    (void)sig;
    void (*isr)(void) = estop_isr;
    bool first = estop_preempted == -1 && pump_on;

    in_isr = 1;
    if (isr != NULL) isr();
    in_isr = 0;

    /* If the ISR woke a task and requested a context switch, that task has
     * already run (here: switched the pump off) by the time the ISR returns
     * to the task it interrupted. Without the switch request it runs later. */
    if (first) estop_preempted = !pump_on;
}

void hw_init(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        long *target = NULL;
        long value;
        char *end;
        if (i + 1 >= argc) usage(argv[0], argv[i]);
        value = strtol(argv[i + 1], &end, 10);
        if (*end != '\0' || value < 0) usage(argv[0], argv[i]);
        if (strcmp(argv[i], "--run-ms") == 0 && value > 0) run_ms = (int)value;
        else if (strcmp(argv[i], "--estop-at") == 0) target = &estop_at;
        else if (strcmp(argv[i], "--sensor-stall-at") == 0) target = &stall_at;
        else usage(argv[0], argv[i]);
        if (target) *target = value;
        i++;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = estop_signal_handler;
    sigfillset(&sa.sa_mask); /* nothing interrupts an ISR, like the port's tick */
    sigaction(SIGUSR2, &sa, NULL);

    setvbuf(stdout, NULL, _IOLBF, 0);
    atexit(report);
}

int hw_run_ms(void) { return run_ms; }

void hw_estop_attach_isr(void (*isr)(void)) { estop_isr = isr; }

bool hw_estop_pressed(void) { return estop_pressed; }

void hw_pump_set(bool on)
{
    must_not_be_in_isr("hw_pump_set()");
    taskENTER_CRITICAL();
    if (on && !pump_on) pump_starts++;
    pump_on = on;
    taskEXIT_CRITICAL();
}

int hw_read_level(void)
{
    must_not_be_in_isr("hw_read_level()");
    TickType_t start = xTaskGetTickCount();
    unsigned long n;
    bool stalled = stall_at >= 0 && (long)start >= stall_at;

    taskENTER_CRITICAL();
    n = adc_reads++;
    if (!stalled) {
        if (adc_last_start >= 0) {
            long gap = (long)start - adc_last_start;
            if (adc_period_min < 0 || gap < adc_period_min) adc_period_min = gap;
            if (gap > adc_period_max) adc_period_max = gap;
        }
        adc_last_start = (long)start;
    }
    taskEXIT_CRITICAL();

    if (stalled) {
        for (;;) vTaskDelay(portMAX_DELAY); /* end-of-conversion never comes */
    }

    vTaskDelay(2 + (TickType_t)((n * 7) % 5)); /* the conversion: 2..6 ms */

    taskENTER_CRITICAL();
    int value = level;
    taskEXIT_CRITICAL();
    return value;
}

/* Called by the kernel from the tick ISR, once per tick. */
void vApplicationTickHook(void)
{
    hw_ticks++;

    level += INFLOW_PER_MS;
    if (pump_on) level -= PUMP_PER_MS;
    if (level >= LEVEL_MAX) {
        level = LEVEL_MAX;
        overflow_ticks++;
    }
    if (level <= 0) {
        level = 0;
        if (pump_on) dry_run_ticks++;
    }
    if (level > max_level) max_level = level;
    if (estop_pressed && pump_on) estop_pump_ticks++;

    /* The e-stop contacts bounce: three edges, at T, T+1 and T+3. */
    if (estop_at >= 0) {
        long t = (long)hw_ticks;
        if (t == estop_at) estop_pressed = true;
        if (t == estop_at || t == estop_at + 1 || t == estop_at + 3) {
            estop_irqs++;
            kill(getpid(), SIGUSR2); /* pending until the running task unblocks signals */
        }
    }
}

/* Task-only kernel functions, checked against the ISR (see the top of this
 * file). The names are the kernel's real entry points: xQueueSend() and
 * xSemaphoreGive() are macros for xQueueGenericSend(), and so on. */
static void task_only(const char *what, const char *instead)
{
    if (in_isr) {
        fprintf(stderr, "hw: %s called from an ISR: only FromISR APIs are allowed there (%s)\n", what, instead);
        abort();
    }
}

#define NO_WAITING "an ISR must never wait: wake a task and let it wait"

BaseType_t __real_xQueueGenericSend(QueueHandle_t q, const void *item, TickType_t wait, BaseType_t pos);
BaseType_t __wrap_xQueueGenericSend(QueueHandle_t q, const void *item, TickType_t wait, BaseType_t pos)
{
    task_only("xQueueSend() or xSemaphoreGive()", "use xQueueSendFromISR() or xSemaphoreGiveFromISR()");
    return __real_xQueueGenericSend(q, item, wait, pos);
}

BaseType_t __real_xQueueReceive(QueueHandle_t q, void *item, TickType_t wait);
BaseType_t __wrap_xQueueReceive(QueueHandle_t q, void *item, TickType_t wait)
{
    task_only("xQueueReceive()", "use xQueueReceiveFromISR()");
    return __real_xQueueReceive(q, item, wait);
}

BaseType_t __real_xQueueSemaphoreTake(QueueHandle_t q, TickType_t wait);
BaseType_t __wrap_xQueueSemaphoreTake(QueueHandle_t q, TickType_t wait)
{
    task_only("xSemaphoreTake()", "use xSemaphoreTakeFromISR() for a binary semaphore; a mutex can't be taken in an ISR at all");
    return __real_xQueueSemaphoreTake(q, wait);
}

#if configUSE_TASK_NOTIFICATIONS == 1
BaseType_t __real_xTaskGenericNotify(TaskHandle_t task, UBaseType_t index, uint32_t value, eNotifyAction action,
                                     uint32_t *previous);
BaseType_t __wrap_xTaskGenericNotify(TaskHandle_t task, UBaseType_t index, uint32_t value, eNotifyAction action,
                                     uint32_t *previous)
{
    task_only("xTaskNotify() or xTaskNotifyGive()", "use xTaskNotifyFromISR() or vTaskNotifyGiveFromISR()");
    return __real_xTaskGenericNotify(task, index, value, action, previous);
}

BaseType_t __real_xTaskGenericNotifyWait(UBaseType_t index, uint32_t clear_on_entry, uint32_t clear_on_exit,
                                         uint32_t *value, TickType_t wait);
BaseType_t __wrap_xTaskGenericNotifyWait(UBaseType_t index, uint32_t clear_on_entry, uint32_t clear_on_exit,
                                         uint32_t *value, TickType_t wait)
{
    task_only("xTaskNotifyWait()", NO_WAITING);
    return __real_xTaskGenericNotifyWait(index, clear_on_entry, clear_on_exit, value, wait);
}

uint32_t __real_ulTaskGenericNotifyTake(UBaseType_t index, BaseType_t clear, TickType_t wait);
uint32_t __wrap_ulTaskGenericNotifyTake(UBaseType_t index, BaseType_t clear, TickType_t wait)
{
    task_only("ulTaskNotifyTake()", NO_WAITING);
    return __real_ulTaskGenericNotifyTake(index, clear, wait);
}
#endif

void __real_vTaskDelay(TickType_t ticks);
void __wrap_vTaskDelay(TickType_t ticks)
{
    task_only("vTaskDelay()", NO_WAITING);
    __real_vTaskDelay(ticks);
}

TickType_t __real_xTaskGetTickCount(void);
TickType_t __wrap_xTaskGetTickCount(void)
{
    task_only("xTaskGetTickCount()", "use xTaskGetTickCountFromISR()");
    return __real_xTaskGetTickCount();
}

#if configUSE_TIMERS == 1
BaseType_t __real_xTimerGenericCommandFromTask(TimerHandle_t timer, BaseType_t command, TickType_t value,
                                               BaseType_t *woken, TickType_t wait);
BaseType_t __wrap_xTimerGenericCommandFromTask(TimerHandle_t timer, BaseType_t command, TickType_t value,
                                               BaseType_t *woken, TickType_t wait)
{
    task_only("xTimerStart(), xTimerStop() or xTimerReset()", "use the timer's ...FromISR() version");
    return __real_xTimerGenericCommandFromTask(timer, command, value, woken, wait);
}
#endif

void __real_vPortEnterCritical(void);
void __wrap_vPortEnterCritical(void)
{
    task_only("taskENTER_CRITICAL()", "use taskENTER_CRITICAL_FROM_ISR()");
    __real_vPortEnterCritical();
}
