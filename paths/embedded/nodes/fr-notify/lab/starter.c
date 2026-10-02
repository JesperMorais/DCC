#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

/* Wearable step counter: the accelerometer raises EXTI0 for every new sample in its FIFO. */

#define IMU_SILENCE_TIMEOUT pdMS_TO_TICKS(50)
#define SAMPLE_LOG_LEN 32

TaskHandle_t g_imu_task;
uint32_t g_samples;                     /* FIFO samples read */
uint32_t g_timeouts;                    /* times the IMU went silent for 50 ms */
TickType_t g_sample_tick[SAMPLE_LOG_LEN];

/* ---- provided: don't change ---- */
static void read_one_fifo_sample(void) {
    if (g_samples < SAMPLE_LOG_LEN) g_sample_tick[g_samples] = xTaskGetTickCount();
    g_samples++;
    vSimulateWork(1); /* SPI burst read */
}

static void display_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) vSimulateWork(100); /* watch-face animation */
}
/* ---- end of provided code ---- */

/* TODO: replace this binary semaphore with a direct-to-task notification.
 * Review comment: "a burst of FIFO interrupts collapses into one wake-up, and
 * the power manager notifies g_imu_task directly, which this ignores". */
SemaphoreHandle_t g_imu_sem;

void EXTI0_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(g_imu_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void imu_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        if (xSemaphoreTake(g_imu_sem, IMU_SILENCE_TIMEOUT) == pdFALSE) {
            g_timeouts++; /* sensor silent: the real code re-inits the IMU here */
            continue;
        }
        read_one_fifo_sample();
    }
}

void app_main(void) {
    g_imu_sem = xSemaphoreCreateBinary();
    xTaskCreate(imu_task, "imu", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 4, &g_imu_task);
    xTaskCreate(display_task, "display", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL);
}
