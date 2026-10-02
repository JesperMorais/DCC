#ifndef DTS_FREERTOS_SEMPHR_H
#define DTS_FREERTOS_SEMPHR_H
#include "FreeRTOS.h"

typedef struct dts_frt_sem *SemaphoreHandle_t;

SemaphoreHandle_t xSemaphoreCreateBinary(void);                       /* starts EMPTY */
SemaphoreHandle_t xSemaphoreCreateCounting(UBaseType_t max, UBaseType_t initial);
SemaphoreHandle_t xSemaphoreCreateMutex(void);                        /* with priority inheritance */
BaseType_t xSemaphoreTake(SemaphoreHandle_t s, TickType_t timeout);
BaseType_t xSemaphoreGive(SemaphoreHandle_t s);
BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t s, BaseType_t *higher_priority_task_woken);
UBaseType_t uxSemaphoreGetCount(SemaphoreHandle_t s);
void vSemaphoreDelete(SemaphoreHandle_t s);
#endif
