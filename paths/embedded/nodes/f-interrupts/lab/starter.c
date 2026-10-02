#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "mcu.h"

#define RX_BUF_SIZE 16u /* a power of two; holds RX_BUF_SIZE - 1 bytes */

static uint8_t rx_buf[RX_BUF_SIZE];
static volatile uint32_t rx_head; /* written only by the ISR */
static volatile uint32_t rx_tail; /* written only by uart_read */
static volatile uint32_t rx_dropped;
static volatile uint32_t rx_overruns;

void uart_init(void) {
    /* TODO: empty the ring, zero the counters, enable UE | RE | RXNEIE */
}

void USART1_IRQHandler(void) {
    /* TODO: RXNE? read DR, acknowledge, check ORE, push or drop */
}

bool uart_read(uint8_t *out) {
    (void)out;
    (void)rx_buf;
    return false;
}

size_t uart_available(void) {
    return 0;
}

uint32_t uart_dropped(void) {
    return rx_dropped;
}

uint32_t uart_overruns(void) {
    return rx_overruns;
}
