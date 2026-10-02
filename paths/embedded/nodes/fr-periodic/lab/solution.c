#include "FreeRTOS.h"
#include "task.h"

/* Pump pressure controller: a 10 ms control loop that must stay on its grid. */

#define CONTROL_PERIOD pdMS_TO_TICKS(10)
#define MAX_LOGGED 128

uint32_t g_cycles;                  /* completed control cycles */
uint32_t g_overruns;                /* cycles that finished after the next deadline */
TickType_t g_cycle_start[MAX_LOGGED]; /* tick at which each cycle started */

/* ---- provided: don't change ---- */
uint32_t g_slow_cycle = UINT32_MAX; /* tests make this cycle slow */
TickType_t g_slow_ticks = 14;

static TickType_t control_work_ticks(uint32_t cycle) {
    return cycle == g_slow_cycle ? g_slow_ticks : 2; /* read sensor, PID, write PWM */
}

static void diagnostics_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) vSimulateWork(50); /* CRC over flash, never sleeps */
}
/* ---- end of provided code ---- */

static void control_task(void *pvParameters) {
    (void)pvParameters;
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        if (g_cycles < MAX_LOGGED) g_cycle_start[g_cycles] = xTaskGetTickCount();
        vSimulateWork(control_work_ticks(g_cycles));
        g_cycles++;
        if (xTaskDelayUntil(&last_wake, CONTROL_PERIOD) == pdFALSE) g_overruns++;
    }
}

void app_main(void) {
    xTaskCreate(control_task, "control", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 5, NULL);
    xTaskCreate(diagnostics_task, "diagnostics", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL);
}
