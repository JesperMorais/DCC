/* The PC implementation of hal.h, plus the simulator hooks in sim.h (given). */
#include <stdio.h>
#include <string.h>

#include "hal.h"
#include "sim.h"
#include "uart.h"

static uart_regs_t uart;
static dev_regs_t dev;

static char tx[8192];
static size_t tx_len;
static bool to_stdout;

uart_regs_t *hal_uart(void) { return &uart; }
dev_regs_t *hal_dev(void) { return &dev; }

void hal_putc(char c) {
    if (tx_len < sizeof tx - 1) {
        tx[tx_len++] = c;
        tx[tx_len] = '\0';
    }
    if (to_stdout) putchar(c);
}

void sim_reset(void) {
    memset((void *)&uart, 0, sizeof uart);
    memset((void *)&dev, 0, sizeof dev);
    uart.SR = UART_SR_TXE;
    dev.ID = DEV_ID_VALUE;
    sim_tx_clear();
}

void sim_uart_rx_masked(uint8_t byte) {
    if (uart.SR & UART_SR_RXNE) {   /* nobody read the last byte: the new one is lost */
        uart.SR |= UART_SR_ORE;
        return;
    }
    uart.DR = byte;
    uart.SR |= UART_SR_RXNE;
}

void sim_uart_irq(void) {
    if (uart.SR & UART_SR_RXNE) uart_rx_isr();
}

void sim_uart_rx(uint8_t byte) {
    sim_uart_rx_masked(byte);
    sim_uart_irq();
}

void sim_gpio_drive(uint32_t pins) { dev.GPIO_IN = pins & 0xFFFFu; }

const char *sim_tx(void) { return tx; }

void sim_tx_clear(void) {
    tx_len = 0;
    tx[0] = '\0';
}

void sim_stdout(bool on) { to_stdout = on; }
