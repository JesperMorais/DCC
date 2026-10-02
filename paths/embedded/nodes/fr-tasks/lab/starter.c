#include "FreeRTOS.h"
#include "task.h"

/* Conveyor safety controller. The design below was prototyped on the neutral
 * rtos.h kernel from Fundamentals. Port it to FreeRTOS.
 *
 *   Neutral design                                             stack (bytes)
 *   rtos_task_create("boot",   boot_task,   NULL,          30);   1024
 *   rtos_task_create("estop",  estop_task,  NULL,          20);    512
 *   rtos_task_create("sensor", sensor_task, &g_sensor_cfg, 10);   1024
 *   rtos_task_create("logger", logger_task, NULL,           2);   2048
 */

typedef struct {
    uint32_t period_ms; /* how often to sample */
    uint32_t samples;   /* how many samples taken so far */
} sensor_cfg_t;

sensor_cfg_t g_sensor_cfg = {.period_ms = 10, .samples = 0};
bool g_boot_done;
uint32_t g_estop_polls;
uint32_t g_log_lines;

TaskHandle_t g_estop_handle;
TaskHandle_t g_sensor_handle;
TaskHandle_t g_logger_handle;

/* TODO: port each task body to the FreeRTOS API (vSimulateWork, vTaskDelay,
 * pdMS_TO_TICKS, vTaskDelete) and fix the function signatures. */

static void boot_task(void *arg) {
    (void)arg;
    rtos_busy(3); /* clocks, GPIO, sensor self-test */
    g_boot_done = true;
    /* the neutral kernel let a task simply return... */
}

static void estop_task(void *arg) {
    (void)arg;
    for (;;) {
        rtos_busy(1); /* poll the emergency-stop chain */
        g_estop_polls++;
        rtos_delay(5);
    }
}

static void sensor_task(void *arg) {
    (void)arg; /* TODO: the config arrives through the task parameter */
    for (;;) {
        rtos_busy(2); /* read the belt-speed sensor */
        g_sensor_cfg.samples++;
        rtos_delay(10);
    }
}

static void logger_task(void *arg) {
    (void)arg;
    for (;;) {
        rtos_busy(4); /* format and write a log line */
        g_log_lines++;
        rtos_delay(20);
    }
}

BaseType_t app_main(void) {
    /* TODO: create the four tasks with xTaskCreate(), with valid FreeRTOS
     * priorities, stack depths in words, and the handles stored. */
    (void)boot_task;
    (void)estop_task;
    (void)sensor_task;
    (void)logger_task;
    return pdFAIL;
}
