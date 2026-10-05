#include "hal.h"
#include "ringbuf.h"
#include "uart.h"

static uint8_t rx_storage[UART_RX_SIZE];
static ringbuf_t rx_ring;
static volatile uint32_t overruns;   /* written only by the ISR */

void uart_init(void) {
    (void)rb_init(&rx_ring, rx_storage, UART_RX_SIZE);
    overruns = 0;
}

void uart_rx_isr(void) {
    uart_regs_t *uart = hal_uart();
    uint32_t sr = uart->SR;               /* read once */
    if (!(sr & UART_SR_RXNE)) return;     /* not ours */
    if (sr & UART_SR_ORE) overruns++;
    uint8_t byte = (uint8_t)uart->DR;
    uart->SR &= ~(UART_SR_RXNE | UART_SR_ORE);
    rb_put(&rx_ring, byte);               /* counts the drop if full */
}

bool uart_getc(uint8_t *out) {
    return rb_get(&rx_ring, out);
}

uint32_t uart_rx_dropped(void) {
    return rx_ring.dropped;
}

uint32_t uart_overruns(void) {
    return overruns;
}
