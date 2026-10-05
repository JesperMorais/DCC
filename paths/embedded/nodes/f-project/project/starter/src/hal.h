/* The hardware abstraction layer (given). The core (ring buffer, UART driver,
 * line editor, shell, commands) only touches hardware through these three
 * functions, so it builds and tests on your PC. To run it on a real board you
 * write another implementation of this header; hal_host.c is the PC one. */
#ifndef HAL_H
#define HAL_H

#include "periph.h"

uart_regs_t *hal_uart(void);   /* the UART register block */
dev_regs_t *hal_dev(void);     /* the device register block (LED, GPIO, ID) */
void hal_putc(char c);         /* transmit one character */

#endif
