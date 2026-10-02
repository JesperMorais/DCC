#include "mcu.h"

GPIO_TypeDef dts_gpioa, dts_gpiob, dts_gpioc;
USART_TypeDef dts_usart1 = {.SR = USART_SR_TXE | USART_SR_TC};

__attribute__((weak)) void USART1_IRQHandler(void) {}
__attribute__((weak)) void SysTick_Handler(void) {}

static bool irq_on = true;
static unsigned pending_uart, pending_tick;

bool dts_irq_enabled(void) { return irq_on; }
void __disable_irq(void) { irq_on = false; }
void __enable_irq(void) {
    irq_on = true;
    while (pending_uart) {
        pending_uart--;
        USART1_IRQHandler();
    }
    while (pending_tick) {
        pending_tick--;
        SysTick_Handler();
    }
}

void sim_uart_receive(uint8_t byte) {
    if (dts_usart1.SR & USART_SR_RXNE) dts_usart1.SR |= USART_SR_ORE; /* the previous byte was never read */
    dts_usart1.DR = byte;
    dts_usart1.SR |= USART_SR_RXNE;
    if (dts_usart1.CR1 & USART_CR1_RXNEIE) {
        if (irq_on) USART1_IRQHandler();
        else pending_uart = 1; /* a single pending flag, like a real NVIC */
    }
}

void sim_systick(unsigned count) {
    for (unsigned i = 0; i < count; i++) {
        if (irq_on) SysTick_Handler();
        else pending_tick = 1;
    }
}

void sim_gpio_set_input(GPIO_TypeDef *port, unsigned pin, bool high) {
    if (high) port->IDR |= 1u << pin;
    else port->IDR &= ~(1u << pin);
}

void sim_gpio_apply_bsrr(GPIO_TypeDef *port) {
    uint32_t v = port->BSRR;
    port->ODR = (port->ODR & ~(v >> 16)) | (v & 0xFFFFu); /* set wins over reset */
    port->BSRR = 0;
}
