/* The simulator's side of the hardware (given). Used by main.c and the tests,
 * never by the core. */
#ifndef SIM_H
#define SIM_H

#include <stdbool.h>
#include <stdint.h>

void sim_reset(void);                   /* registers to reset values (SR = TXE, ID = DEV_ID_VALUE), TX capture cleared */
void sim_uart_rx(uint8_t byte);         /* a byte arrives with interrupts enabled: latch it, then run uart_rx_isr() */
void sim_uart_rx_masked(uint8_t byte);  /* a byte arrives with interrupts masked: latch it only (or flag ORE if one is still waiting) */
void sim_uart_irq(void);                /* interrupts unmasked again: runs uart_rx_isr() if RXNE is set */
void sim_gpio_drive(uint32_t pins);     /* the outside world sets GPIO_IN */

const char *sim_tx(void);               /* everything transmitted since the last reset/clear */
void sim_tx_clear(void);
void sim_stdout(bool on);               /* also copy transmitted characters to stdout (main.c turns this on) */

#endif
