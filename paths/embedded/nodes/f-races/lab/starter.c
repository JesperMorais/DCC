#include <stdbool.h>
#include <stdint.h>
#include "mcu.h"

#define EVT_TICK (1u << 0)
#define EVT_RX (1u << 1)

/* Shared between the ISRs and the main loop. */
volatile uint32_t ticks_lo, ticks_hi; /* 64-bit uptime as two halves, like on a 32-bit MCU */
volatile uint32_t pending_events;     /* EVT_* bits set by the ISRs */

/* Defined by the tests: an interrupt fires here. Keep these calls where they are. */
void interrupt_window(void);

/* ---- interrupt handlers: these are correct, leave them alone ---- */
void SysTick_Handler(void) {
    if (++ticks_lo == 0) ticks_hi++;
    pending_events |= EVT_TICK;
}

void USART1_IRQHandler(void) {
    if (!(USART1->SR & USART_SR_RXNE)) return;
    (void)USART1->DR;
    USART1->SR &= ~USART_SR_RXNE;
    pending_events |= EVT_RX;
}

/* ---- main-loop code: three races live below ---- */
uint64_t uptime_ticks(void) {
    uint32_t lo = ticks_lo;
    interrupt_window();
    uint32_t hi = ticks_hi;
    return ((uint64_t)hi << 32) | lo;
}

uint32_t events_take(void) {
    uint32_t ev = pending_events;
    interrupt_window();
    pending_events = 0;
    return ev;
}

void events_clear(uint32_t mask) {
    uint32_t ev = pending_events;
    interrupt_window();
    pending_events = ev & ~mask;
}
