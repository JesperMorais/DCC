/* Soil-moisture station: ADC ISR -> sample_q -> filter task -> log_q -> logger task. */
#include <stdint.h>
#include "rtos.h"

#define SAMPLE_Q_LEN 8 /* samples buffered between the ISR and the filter */
#define LOG_Q_LEN 4    /* averages buffered between the filter and the logger */
#define WINDOW 4       /* samples per average */

rtos_queue_t *sample_q; /* uint16_t raw samples */
rtos_queue_t *log_q;    /* int32_t averages */
unsigned samples_dropped;

/* Provided by the board (the tests). */
uint16_t adc_read(void);     /* read the conversion result (ISR-safe) */
void log_line(int32_t avg); /* write a line to the SD card: slow, task context only */

/* ADC "conversion complete" interrupt. */
void adc_isr(void) {
    uint16_t sample = adc_read();
    rtos_queue_send(sample_q, &sample, RTOS_WAIT_FOREVER); /* TODO: what if the queue is full? */
}

static void filter_task(void *arg) {
    (void)arg;
    for (;;) {
        int32_t sum = 0;
        for (int i = 0; i < WINDOW; i++) {
            uint16_t sample;
            rtos_queue_receive(sample_q, &sample, RTOS_WAIT_FOREVER);
            sum += sample;
        }
        rtos_busy(1); /* calibration curve */
        int32_t avg = sum / WINDOW;
        (void)avg; /* TODO: hand the average to the logger */
    }
}

static void logger_task(void *arg) {
    (void)arg;
    for (;;) {
        rtos_delay(100); /* TODO: wait for averages and log_line() each one, in order */
    }
}

void station_start(void) {
    sample_q = rtos_queue_create(SAMPLE_Q_LEN, sizeof(uint16_t));
    log_q = rtos_queue_create(LOG_Q_LEN, sizeof(int32_t));
    rtos_task_create("filter", filter_task, NULL, 3);
    rtos_task_create("logger", logger_task, NULL, 1);
}
