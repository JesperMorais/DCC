#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "mcu.h"

#define RX_BUF_SIZE 16u /* a power of two; holds RX_BUF_SIZE - 1 bytes */
#define RX_MASK (RX_BUF_SIZE - 1u)

static uint8_t rx_buf[RX_BUF_SIZE];
static volatile uint32_t rx_head; /* written only by the ISR */
static volatile uint32_t rx_tail; /* written only by uart_read */
static volatile uint32_t rx_dropped;
static volatile uint32_t rx_overruns;

void uart_init(void) {
    rx_head = 0;
    rx_tail = 0;
    rx_dropped = 0;
    rx_overruns = 0;
    USART1->CR1 |= USART_CR1_UE | USART_CR1_RE | USART_CR1_RXNEIE;
}

void USART1_IRQHandler(void) {
    uint32_t sr = USART1->SR;
    if (!(sr & USART_SR_RXNE)) {
        return; /* not a receive interrupt */
    }
    uint8_t byte = (uint8_t)USART1->DR;
    if (sr & USART_SR_ORE) {
        rx_overruns++; /* the hardware lost a byte before we got here */
    }
    USART1->SR &= ~(USART_SR_RXNE | USART_SR_ORE); /* acknowledge, even if we drop it */

    uint32_t next = (rx_head + 1u) & RX_MASK;
    if (next == rx_tail) {
        rx_dropped++; /* full: drop the newest, never block */
        return;
    }
    rx_buf[rx_head] = byte; /* fill the slot first... */
    rx_head = next;         /* ...then publish it */
}

bool uart_read(uint8_t *out) {
    uint32_t tail = rx_tail;
    if (tail == rx_head) {
        return false;
    }
    *out = rx_buf[tail];
    rx_tail = (tail + 1u) & RX_MASK;
    return true;
}

size_t uart_available(void) {
    return (rx_head - rx_tail) & RX_MASK;
}

uint32_t uart_dropped(void) {
    return rx_dropped;
}

uint32_t uart_overruns(void) {
    return rx_overruns;
}
