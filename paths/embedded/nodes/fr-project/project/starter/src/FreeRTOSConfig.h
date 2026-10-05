/* FreeRTOSConfig.h: kernel configuration for the POSIX (Linux) port.
 *
 * Given. You may change values while experimenting, but the tests assume
 * a 1 ms tick, preemption, and the timer daemon at priority 6. */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Scheduling */
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configTICK_RATE_HZ                      1000 /* 1 tick = 1 ms */
#define configMAX_PRIORITIES                    8    /* 0 (idle) .. 7 (most urgent) */
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_16_BIT_TICKS                  0
#define configMAX_TASK_NAME_LEN                 16

/* Memory. On the POSIX port every task really runs on its own pthread
 * stack; the FreeRTOS "stack" only holds the port's bookkeeping, so a small
 * value is fine here (unlike on a microcontroller). heap_4 is linked. */
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0
#define configMINIMAL_STACK_SIZE                256
#define configTOTAL_HEAP_SIZE                   ( 512 * 1024 )
#define configCHECK_FOR_STACK_OVERFLOW          0 /* meaningless on pthread stacks */
#define configUSE_MALLOC_FAILED_HOOK            1

/* Features */
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   1
#define configQUEUE_REGISTRY_SIZE               0
#define configUSE_QUEUE_SETS                    0
#define configUSE_CO_ROUTINES                   0
#define configUSE_TRACE_FACILITY                0
#define configGENERATE_RUN_TIME_STATS           0

/* Software timers: callbacks run in the "Tmr Svc" daemon task. */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               6
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE

/* Hooks. hw.c implements the tick hook: it is how the simulated hardware
 * sees time pass. hooks.c implements the others. */
#define configUSE_IDLE_HOOK                     1
#define configUSE_TICK_HOOK                     1

/* API functions to include */
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskDelayUntil                 1
#define INCLUDE_vTaskSuspend                    1 /* portMAX_DELAY waits forever */
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_uxTaskGetStackHighWaterMark     0
#define INCLUDE_xTimerPendFunctionCall          0

/* A failed configASSERT() prints where it happened and aborts. */
void vAssertCalled( const char * file, unsigned long line );
#define configASSERT( x )    do { if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ ); } while( 0 )

#endif /* FREERTOS_CONFIG_H */
