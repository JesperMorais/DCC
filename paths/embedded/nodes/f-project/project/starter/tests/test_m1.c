/* Milestone 1: the ring buffer and the RX interrupt handler. */
#include "check.h"
#include "hal.h"
#include "ringbuf.h"
#include "sim.h"
#include "uart.h"

static void ring_checks_size(void) {
    uint8_t s[64];
    ringbuf_t rb;
    CHECK(!rb_init(&rb, s, 0));
    CHECK(!rb_init(&rb, s, 1));
    CHECK(!rb_init(&rb, s, 3));
    CHECK(!rb_init(&rb, s, 48));
    CHECK(rb_init(&rb, s, 2));
    CHECK(rb_init(&rb, s, 64));
}

static void ring_is_fifo(void) {
    uint8_t s[8], b = 0;
    ringbuf_t rb;
    CHECK(rb_init(&rb, s, 8));
    CHECK(!rb_get(&rb, &b));
    CHECK(rb_put(&rb, 'a'));
    CHECK(rb_put(&rb, 'b'));
    CHECK(rb_put(&rb, 'c'));
    CHECK_INT(rb_count(&rb), 3);
    CHECK(rb_get(&rb, &b));
    CHECK_INT(b, 'a');
    CHECK(rb_get(&rb, &b));
    CHECK_INT(b, 'b');
    CHECK(rb_get(&rb, &b));
    CHECK_INT(b, 'c');
    CHECK(!rb_get(&rb, &b));
    CHECK_INT(rb_count(&rb), 0);
}

static void ring_full_drops_newest(void) {
    uint8_t s[8], b = 0;
    ringbuf_t rb;
    CHECK(rb_init(&rb, s, 8));
    for (int i = 0; i < 7; i++) CHECK(rb_put(&rb, (uint8_t)(10 + i)));
    CHECK_INT(rb_count(&rb), 7);
    CHECK(!rb_put(&rb, 99));
    CHECK(!rb_put(&rb, 98));
    CHECK_INT(rb.dropped, 2);
    CHECK_INT(rb_count(&rb), 7);
    for (int i = 0; i < 7; i++) {
        CHECK(rb_get(&rb, &b));
        CHECK_INT(b, 10 + i);
    }
    CHECK(!rb_get(&rb, &b));
    CHECK(rb_put(&rb, 42));
    CHECK_INT(rb.dropped, 2);
}

static void ring_wraps_around(void) {
    uint8_t s[4], b = 0;
    ringbuf_t rb;
    CHECK(rb_init(&rb, s, 4));
    CHECK(rb_put(&rb, 0));
    CHECK(rb_put(&rb, 1));
    for (int i = 0; i < 1000; i++) {   /* always two bytes in flight */
        CHECK(rb_put(&rb, (uint8_t)(i + 2)));
        CHECK(rb_get(&rb, &b));
        CHECK_INT(b, (uint8_t)i);
        CHECK_INT(rb_count(&rb), 2);
    }
    CHECK_INT(rb.dropped, 0);
}

static void ring_init_resets(void) {
    uint8_t s[4], b = 0;
    ringbuf_t rb;
    CHECK(rb_init(&rb, s, 4));
    for (int i = 0; i < 6; i++) rb_put(&rb, 1);
    CHECK(rb_init(&rb, s, 4));
    CHECK_INT(rb_count(&rb), 0);
    CHECK_INT(rb.dropped, 0);
    CHECK(!rb_get(&rb, &b));
}

static void isr_moves_bytes_to_main(void) {
    uint8_t b = 0;
    sim_reset();
    uart_init();
    sim_uart_rx('o');
    sim_uart_rx('k');
    CHECK(uart_getc(&b));
    CHECK_INT(b, 'o');
    CHECK(uart_getc(&b));
    CHECK_INT(b, 'k');
    CHECK(!uart_getc(&b));
}

static void isr_clears_rxne_only(void) {
    sim_reset();
    uart_init();
    sim_uart_rx('x');
    CHECK_HEX(hal_uart()->SR, UART_SR_TXE);   /* RXNE cleared, TXE untouched */
}

static void isr_ignores_spurious_interrupt(void) {
    uint8_t b = 0;
    sim_reset();
    uart_init();
    hal_uart()->DR = 'z';   /* stale data, but RXNE is not set */
    uart_rx_isr();
    CHECK(!uart_getc(&b));
    CHECK_HEX(hal_uart()->SR, UART_SR_TXE);
}

static void isr_counts_drops_when_full(void) {
    uint8_t b = 0;
    sim_reset();
    uart_init();
    for (int i = 0; i < 70; i++) sim_uart_rx((uint8_t)('A' + i % 26));
    CHECK_INT(uart_rx_dropped(), 70 - (UART_RX_SIZE - 1));
    CHECK_HEX(hal_uart()->SR, UART_SR_TXE);
    int n = 0;
    while (uart_getc(&b)) {
        CHECK_INT(b, 'A' + n % 26);
        n++;
        if (n > 100) break;
    }
    CHECK_INT(n, UART_RX_SIZE - 1);
}

static void isr_counts_overrun(void) {
    uint8_t b = 0;
    sim_reset();
    uart_init();
    sim_uart_rx_masked('a');   /* interrupts masked for too long... */
    sim_uart_rx_masked('b');   /* ...so 'b' is lost and ORE is set */
    CHECK(hal_uart()->SR & UART_SR_ORE);
    sim_uart_irq();
    CHECK(uart_getc(&b));
    CHECK_INT(b, 'a');
    CHECK(!uart_getc(&b));
    CHECK_INT(uart_overruns(), 1);
    CHECK_HEX(hal_uart()->SR, UART_SR_TXE);   /* RXNE and ORE cleared */
}

static void uart_init_resets(void) {
    uint8_t b = 0;
    sim_reset();
    uart_init();
    for (int i = 0; i < 70; i++) sim_uart_rx('q');
    sim_uart_rx_masked('a');
    sim_uart_rx_masked('b');
    sim_uart_irq();
    uart_init();
    CHECK_INT(uart_rx_dropped(), 0);
    CHECK_INT(uart_overruns(), 0);
    CHECK(!uart_getc(&b));
}

void suite_m1(void) {
    RUN(ring_checks_size, "m1: rb_init accepts only powers of two >= 2");
    RUN(ring_is_fifo, "m1: bytes come out in the order they went in");
    RUN(ring_full_drops_newest, "m1: a full ring holds size-1 bytes and counts the dropped newest");
    RUN(ring_wraps_around, "m1: the indices wrap around");
    RUN(ring_init_resets, "m1: rb_init empties a used ring");
    RUN(isr_moves_bytes_to_main, "m1: the ISR moves received bytes to uart_getc");
    RUN(isr_clears_rxne_only, "m1: the ISR clears RXNE and leaves the other SR bits alone");
    RUN(isr_ignores_spurious_interrupt, "m1: the ISR does nothing when RXNE is not set");
    RUN(isr_counts_drops_when_full, "m1: a full receive ring drops and counts the extra bytes");
    RUN(isr_counts_overrun, "m1: the ISR counts and clears an overrun");
    RUN(uart_init_resets, "m1: uart_init empties the ring and zeroes the counters");
}
