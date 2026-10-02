#ifndef DTS_FREERTOS_TIMERS_H
#define DTS_FREERTOS_TIMERS_H
#include "FreeRTOS.h"

typedef struct dts_frt_timer *TimerHandle_t;
typedef void (*TimerCallbackFunction_t)(TimerHandle_t timer);

/* Callbacks run in the timer service ("daemon") task at configTIMER_TASK_PRIORITY — never block in them. */
TimerHandle_t xTimerCreate(const char *name, TickType_t period, UBaseType_t auto_reload, void *id, TimerCallbackFunction_t cb);
BaseType_t xTimerStart(TimerHandle_t t, TickType_t wait);
BaseType_t xTimerStop(TimerHandle_t t, TickType_t wait);
BaseType_t xTimerReset(TimerHandle_t t, TickType_t wait);
void *pvTimerGetTimerID(TimerHandle_t t);
#endif
