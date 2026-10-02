#ifndef DTS_FREERTOS_QUEUE_H
#define DTS_FREERTOS_QUEUE_H
#include "FreeRTOS.h"

typedef struct dts_frt_queue *QueueHandle_t;

QueueHandle_t xQueueCreate(UBaseType_t length, UBaseType_t item_size);
BaseType_t xQueueSend(QueueHandle_t q, const void *item, TickType_t timeout);
BaseType_t xQueueSendToBack(QueueHandle_t q, const void *item, TickType_t timeout);
BaseType_t xQueueReceive(QueueHandle_t q, void *out, TickType_t timeout);
BaseType_t xQueueSendFromISR(QueueHandle_t q, const void *item, BaseType_t *higher_priority_task_woken);
BaseType_t xQueueSendToBackFromISR(QueueHandle_t q, const void *item, BaseType_t *higher_priority_task_woken);
UBaseType_t uxQueueMessagesWaiting(QueueHandle_t q);
void vQueueDelete(QueueHandle_t q);
#endif
