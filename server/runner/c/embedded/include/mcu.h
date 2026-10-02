/* daily.ts simulated microcontroller (STM32-flavoured registers).
 *
 * Peripherals are plain structs of volatile registers, exactly how vendor headers
 * (CMSIS) expose real hardware. Tests drive the "hardware": sim_uart_receive()
 * plays a byte arriving on the wire and fires USART1_IRQHandler() if you enabled
 * the RX interrupt. __disable_irq()/__enable_irq() really hold interrupts pending. */
#ifndef DTS_MCU_H
#define DTS_MCU_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    volatile uint32_t MODER; /* 2 bits per pin: 00 input, 01 output, 10 alternate, 11 analog */
    volatile uint32_t IDR;   /* input data (read-only on real hardware) */
    volatile uint32_t ODR;   /* output data */
    volatile uint32_t BSRR;  /* write-only: bits 0-15 set, bits 16-31 reset (see sim_gpio_apply_bsrr) */
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t SR;  /* status */
    volatile uint32_t DR;  /* data */
    volatile uint32_t BRR; /* baud rate */
    volatile uint32_t CR1; /* control */
} USART_TypeDef;

extern GPIO_TypeDef dts_gpioa, dts_gpiob, dts_gpioc;
extern USART_TypeDef dts_usart1;
#define GPIOA (&dts_gpioa)
#define GPIOB (&dts_gpiob)
#define GPIOC (&dts_gpioc)
#define USART1 (&dts_usart1)

#define USART_SR_RXNE (1u << 5) /* receive register not empty */
#define USART_SR_TC (1u << 6)   /* transmission complete */
#define USART_SR_TXE (1u << 7)  /* transmit register empty */
#define USART_SR_ORE (1u << 3)  /* overrun: a byte arrived before the last one was read */
#define USART_CR1_RE (1u << 2)
#define USART_CR1_TE (1u << 3)
#define USART_CR1_RXNEIE (1u << 5)
#define USART_CR1_UE (1u << 13)

/* Interrupt handlers you may define (weak defaults do nothing). */
void USART1_IRQHandler(void);
void SysTick_Handler(void);

/* Global interrupt mask, like CMSIS. */
void __disable_irq(void);
void __enable_irq(void);
bool dts_irq_enabled(void);

/* ---- test-side "hardware" ---- */
void sim_uart_receive(uint8_t byte);       /* a byte arrives: DR, RXNE, maybe ORE, then the IRQ */
void sim_systick(unsigned count);          /* fire SysTick_Handler count times */
void sim_gpio_set_input(GPIO_TypeDef *port, unsigned pin, bool high);
void sim_gpio_apply_bsrr(GPIO_TypeDef *port); /* what the silicon does when BSRR is written */

#endif
