/* main.c: create everything, run for --run-ms, then shut down cleanly. */
#include <stdio.h>

#include "app.h"
#include "hw.h"

QueueHandle_t g_readings;
SemaphoreHandle_t g_status_mutex;
plant_status_t g_status;
TaskHandle_t g_safety_task;

static void supervisor_task(void *arg)
{
    (void)arg;
    vTaskDelay(pdMS_TO_TICKS(hw_run_ms()));

    xSemaphoreTake(g_status_mutex, portMAX_DELAY);
    plant_status_t s = g_status; /* copy under the lock, print outside it */
    xSemaphoreGive(g_status_mutex);

    log_last("END samples=%lu pump_starts=%lu max_level=%d estop_irqs=%lu", (unsigned long)s.samples,
             (unsigned long)s.pump_starts, s.max_level_mm, (unsigned long)estop_irq_count());
    vTaskDelete(NULL);
}

static void create(TaskFunction_t fn, const char *name, UBaseType_t prio, TaskHandle_t *handle)
{
    BaseType_t ok = xTaskCreate(fn, name, configMINIMAL_STACK_SIZE, NULL, prio, handle);
    configASSERT(ok == pdPASS);
}

int main(int argc, char **argv)
{
    hw_init(argc, argv);

    log_init();
    g_readings = xQueueCreate(READING_QUEUE_LEN, sizeof(reading_t));
    g_status_mutex = xSemaphoreCreateMutex();
    configASSERT(g_readings != NULL && g_status_mutex != NULL);

    create(log_task, "logger", PRIO_LOGGER, NULL);
    create(sensor_task, "sensor", PRIO_SENSOR, NULL);
    create(control_task, "control", PRIO_CONTROL, NULL);
    create(safety_task, "safety", PRIO_SAFETY, &g_safety_task);
    create(supervisor_task, "supervisor", PRIO_SUPERVISOR, NULL);
    hw_estop_attach_isr(estop_isr);
    watchdog_start();

    vTaskStartScheduler();
    return 0;
}
