/* FreeRTOS API facade over the daily.ts RTOS simulator.
 * Real FreeRTOS names and semantics; 1 tick = 1 ms (configTICK_RATE_HZ = 1000). */
#ifndef DTS_FREERTOS_H
#define DTS_FREERTOS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "rtos.h"
#include "sim.h"

typedef uint32_t TickType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;
typedef uint32_t StackType_t;

#define pdTRUE ((BaseType_t)1)
#define pdFALSE ((BaseType_t)0)
#define pdPASS pdTRUE
#define pdFAIL pdFALSE
#define errQUEUE_FULL pdFALSE
#define errQUEUE_EMPTY pdFALSE
#define portMAX_DELAY ((TickType_t)0xFFFFFFFFu)
#define configTICK_RATE_HZ 1000
#define configMAX_PRIORITIES 8
#define configTIMER_TASK_PRIORITY 6
#define configMINIMAL_STACK_SIZE 128
#define tskIDLE_PRIORITY ((UBaseType_t)0)
#define portTICK_PERIOD_MS ((TickType_t)1)
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))

#define configASSERT(x)                                                                      \
    do {                                                                                     \
        if (!(x)) sim_fail("configASSERT failed: %s (%s line %d)", #x, __FILE__, __LINE__); \
    } while (0)

#define portYIELD_FROM_ISR(x)       \
    do {                            \
        if ((x) != pdFALSE) sim_isr_yield(); \
    } while (0)
#define portEND_SWITCHING_ISR(x) portYIELD_FROM_ISR(x)
#define taskYIELD() rtos_yield()
#define taskENTER_CRITICAL() rtos_enter_critical()
#define taskEXIT_CRITICAL() rtos_exit_critical()

#endif
