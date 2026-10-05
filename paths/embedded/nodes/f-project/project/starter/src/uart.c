#include "hal.h"
#include "ringbuf.h"
#include "uart.h"

void uart_init(void) {
    /* TODO(m1) */
}

void uart_rx_isr(void) {
    /* TODO(m1) */
}

bool uart_getc(uint8_t *out) {
    (void)out;
    return false;   /* TODO(m1) */
}

uint32_t uart_rx_dropped(void) {
    return 0;       /* TODO(m1) */
}

uint32_t uart_overruns(void) {
    return 0;       /* TODO(m1) */
}
