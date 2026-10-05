/* app.h: what the plant controller's modules share. */
#ifndef APP_H
#define APP_H

#include <stdbool.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "plant.h"

/* Priorities: higher is more urgent. The timer daemon is at 6. */
#define PRIO_SAFETY     ( configMAX_PRIORITIES - 1 ) /* above Tmr Svc */
#define PRIO_CONTROL    5
#define PRIO_SENSOR     4
#define PRIO_SUPERVISOR 2
#define PRIO_LOGGER     1

/* Reasons for the safety task to shut the pump off (notification bits). */
#define SHUTDOWN_ESTOP    ( 1u << 0 )
#define SHUTDOWN_WATCHDOG ( 1u << 1 )

extern QueueHandle_t g_readings;
extern SemaphoreHandle_t g_status_mutex;
extern plant_status_t g_status;
extern TaskHandle_t g_safety_task;

/* log.c: every line goes through the logger task, the only task that prints. */
void log_init(void);
void log_task(void *arg);
bool log_at(TickType_t tick, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
bool log_now(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void log_last(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/* plant.c */
void sensor_task(void *arg);
void control_task(void *arg);
void safety_task(void *arg);
void estop_isr(void);
uint32_t estop_irq_count(void);

/* watchdog.c */
void watchdog_start(void);
void watchdog_checkin(void);

#endif /* APP_H */
