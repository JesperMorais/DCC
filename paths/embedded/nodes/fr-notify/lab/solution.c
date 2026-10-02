#include "FreeRTOS.h"
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

void EXTI0_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(g_imu_task, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void imu_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        /* pdFALSE: decrement, so a burst of N gives N wake-ups (counting semaphore). */
        if (ulTaskNotifyTake(pdFALSE, IMU_SILENCE_TIMEOUT) == 0) {
            g_timeouts++; /* sensor silent: the real code re-inits the IMU here */
            continue;
        }
        read_one_fifo_sample();
    }
}

void app_main(void) {
    xTaskCreate(imu_task, "imu", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 4, &g_imu_task);
    xTaskCreate(display_task, "display", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL);
}
