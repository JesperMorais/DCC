#include <stdbool.h>
#include "rtos.h"
#include "sim.h"

volatile bool flash_ready; /* set by the flash controller's "done" interrupt */
volatile unsigned control_runs, blinks, logs_written;

/* ---- leave these two alone ---- */
void control_task(void *arg) {
    (void)arg;
    for (;;) {
        rtos_busy(2); /* read the sensor, drive the heater */
        control_runs++;
        rtos_delay(8);
    }
}

void blink_task(void *arg) {
    (void)arg;
    for (;;) {
        rtos_busy(1); /* toggle the LED */
        blinks++;
        rtos_delay(49);
    }
}

void logger_task(void *arg) {
    (void)arg;
    for (;;) {
        while (!flash_ready) rtos_delay(1); /* BLOCKED between checks: zero CPU */
        flash_ready = false;
        rtos_busy(3); /* write the log entry */
        logs_written++;
        sim_mark("log written");
    }
}

void app_start(void) {
    /* priorities follow deadlines: the tightest deadline is the most urgent */
    rtos_task_create("control", control_task, NULL, 3); /* every 10 ticks, exactly */
    rtos_task_create("blink", blink_task, NULL, 2);     /* every 50 ticks, loosely */
    rtos_task_create("logger", logger_task, NULL, 1);   /* whenever there's time */
}
