/* Milestone 1: the UART receive driver. */
#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stdint.h>

#define UART_RX_SIZE 64u   /* slots in the receive ring */

void uart_init(void);           /* empty receive ring, counters back to 0 */
void uart_rx_isr(void);         /* the RX interrupt handler: see milestone 1 */
bool uart_getc(uint8_t *out);   /* main loop side: next received byte, false if none */
uint32_t uart_rx_dropped(void); /* bytes lost because the ring was full */
uint32_t uart_overruns(void);   /* times the ISR found ORE set */

#endif
