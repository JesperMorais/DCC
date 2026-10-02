/* Vibration sampler for a wind-turbine gearbox.
 * The FFT downstream assumes samples are EXACTLY 10 ticks apart, forever. */
#include <stdint.h>
#include "rtos.h"

#define SAMPLE_PERIOD 10 /* ticks between samples */

uint16_t adc_read(void); /* provided by the board: takes one sample */

/* How long filtering a reading takes. Given: don't change it. */
static rtos_tick_t filter_cost(uint16_t reading) {
    if (reading == 0xFFFF) return 13; /* sensor fault: recalibrate, longer than a period! */
    if (reading > 3000) return 6;     /* spike: run the median filter again */
    return 2;
}

static void sampler_task(void *arg) {
    (void)arg;
    for (;;) {
        uint16_t reading = adc_read();
        rtos_busy(filter_cost(reading));
        /* BUG: "wait 10 ticks" after the work means each period is 10 + work. */
        rtos_delay(SAMPLE_PERIOD);
    }
}

void sampler_start(void) {
    rtos_task_create("sampler", sampler_task, NULL, 2);
}
