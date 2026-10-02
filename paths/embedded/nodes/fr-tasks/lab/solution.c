#include "FreeRTOS.h"
#include "task.h"

/* Conveyor safety controller, ported from the neutral rtos.h design to FreeRTOS. */

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

/* Priorities: the neutral design used 30 / 20 / 10 / 2. FreeRTOS here allows
 * 0..configMAX_PRIORITIES-1 (0..7), and 0 belongs to the idle task. */
#define PRIO_BOOT (tskIDLE_PRIORITY + 7)
#define PRIO_ESTOP (tskIDLE_PRIORITY + 5)
#define PRIO_SENSOR (tskIDLE_PRIORITY + 3)
#define PRIO_LOGGER (tskIDLE_PRIORITY + 1)

/* Stack depth is in WORDS (4 bytes on a Cortex-M), not bytes. */
#define STACK_BOOT (1024 / sizeof(StackType_t))
#define STACK_ESTOP (512 / sizeof(StackType_t))
#define STACK_SENSOR (1024 / sizeof(StackType_t))
#define STACK_LOGGER (2048 / sizeof(StackType_t))

static void boot_task(void *pvParameters) {
    (void)pvParameters;
    vSimulateWork(3); /* clocks, GPIO, sensor self-test */
    g_boot_done = true;
    vTaskDelete(NULL); /* a FreeRTOS task must never return: delete yourself */
}

static void estop_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        vSimulateWork(1); /* poll the emergency-stop chain */
        g_estop_polls++;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

static void sensor_task(void *pvParameters) {
    sensor_cfg_t *cfg = pvParameters;
    for (;;) {
        vSimulateWork(2); /* read the belt-speed sensor */
        cfg->samples++;
        vTaskDelay(pdMS_TO_TICKS(cfg->period_ms));
    }
}

static void logger_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        vSimulateWork(4); /* format and write a log line */
        g_log_lines++;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

BaseType_t app_main(void) {
    if (xTaskCreate(boot_task, "boot", STACK_BOOT, NULL, PRIO_BOOT, NULL) != pdPASS) return pdFAIL;
    if (xTaskCreate(estop_task, "estop", STACK_ESTOP, NULL, PRIO_ESTOP, &g_estop_handle) != pdPASS) return pdFAIL;
    if (xTaskCreate(sensor_task, "sensor", STACK_SENSOR, &g_sensor_cfg, PRIO_SENSOR, &g_sensor_handle) != pdPASS) return pdFAIL;
    if (xTaskCreate(logger_task, "logger", STACK_LOGGER, NULL, PRIO_LOGGER, &g_logger_handle) != pdPASS) return pdFAIL;
    return pdPASS;
}
