/* The simulated hardware: two register blocks and their bits (given).
 *
 * On a real chip these structs sit on top of fixed addresses. Here the host
 * HAL (hal_host.c) keeps them in RAM, and the tests poke them through sim.h. */
#ifndef PERIPH_H
#define PERIPH_H

#include <stddef.h>
#include <stdint.h>

/* ---- UART -------------------------------------------------------------
 * Laid out like the first two registers of an STM32F4 USART, same bits. */
typedef struct {
    volatile uint32_t SR;   /* 0x00 status */
    volatile uint32_t DR;   /* 0x04 data: a received byte is in bits 0..7 */
} uart_regs_t;

#define UART_SR_ORE   (1u << 3)   /* overrun: a byte arrived while RXNE was still set, and was lost */
#define UART_SR_RXNE  (1u << 5)   /* a received byte is waiting in DR */
#define UART_SR_TXE   (1u << 7)   /* transmitter empty. Owned by the TX side: leave it alone */
/* On this UART, RXNE and ORE are cleared by writing 0 to them.
 * Writing 1 to a flag that is set leaves it set. */

/* ---- The device: an LED, 16 GPIO pins and an ID register ---------------
 *
 *  offset  name      access  contents
 *  0x00    CTRL      rw      bit 0 LED on, bits 4..7 BLINK rate (0 = steady,
 *                            1..15 Hz). All other bits are reserved: keep them
 *                            as they are when you change a field.
 *  0x04    GPIO_OUT  rw      bits 0..15: the level we drive on pins 0..15
 *  0x08    GPIO_IN   ro      bits 0..15: the level the outside world drives
 *  0x0C    ID        ro      always DEV_ID_VALUE
 */
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t GPIO_OUT;
    volatile uint32_t GPIO_IN;
    volatile uint32_t ID;
} dev_regs_t;

_Static_assert(offsetof(dev_regs_t, GPIO_OUT) == 0x04, "register map");
_Static_assert(offsetof(dev_regs_t, GPIO_IN) == 0x08, "register map");
_Static_assert(offsetof(dev_regs_t, ID) == 0x0C, "register map");

#define DEV_CTRL_LED        (1u << 0)
#define DEV_CTRL_BLINK_Pos  4u
#define DEV_CTRL_BLINK_Msk  (0xFu << DEV_CTRL_BLINK_Pos)

#define DEV_GPIO_PINS  16u
#define DEV_ID_VALUE   0xC0DE0042u

#endif
