#ifndef DTS_FREERTOS_TASK_H
#define DTS_FREERTOS_TASK_H
#include "FreeRTOS.h"

typedef rtos_task_t *TaskHandle_t;
typedef void (*TaskFunction_t)(void *);

BaseType_t xTaskCreate(TaskFunction_t fn, const char *name, uint32_t stack_depth, void *param, UBaseType_t priority, TaskHandle_t *created);
void vTaskDelete(TaskHandle_t task); /* NULL = the calling task */
void vTaskDelay(TickType_t ticks);
void vTaskDelayUntil(TickType_t *previous_wake, TickType_t period);
BaseType_t xTaskDelayUntil(TickType_t *previous_wake, TickType_t period);
TickType_t xTaskGetTickCount(void);
TickType_t xTaskGetTickCountFromISR(void);
TaskHandle_t xTaskGetCurrentTaskHandle(void);
UBaseType_t uxTaskPriorityGet(TaskHandle_t task);
void vTaskPrioritySet(TaskHandle_t task, UBaseType_t priority);
const char *pcTaskGetName(TaskHandle_t task);

/* Direct-to-task notifications */
BaseType_t xTaskNotifyGive(TaskHandle_t task);
void vTaskNotifyGiveFromISR(TaskHandle_t task, BaseType_t *higher_priority_task_woken);
uint32_t ulTaskNotifyTake(BaseType_t clear_on_exit, TickType_t timeout);

/* Simulated CPU work (not part of FreeRTOS): burn `ticks` of CPU time. */
#define vSimulateWork(ticks) rtos_busy(ticks)
#endif
