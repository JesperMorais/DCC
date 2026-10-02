#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "mcu.h"

static void send(const char *s) {
    while (*s) sim_uart_receive((uint8_t)*s++);
}

TEST(init_enables_the_receiver_and_its_interrupt) {
    USART1->CR1 = USART_CR1_TE; /* the transmitter was already set up */
    uart_init();
    EXPECT_TRUE(USART1->CR1 & USART_CR1_UE);
    EXPECT_TRUE(USART1->CR1 & USART_CR1_RE);
    EXPECT_TRUE(USART1->CR1 & USART_CR1_RXNEIE);
    EXPECT_TRUE(USART1->CR1 & USART_CR1_TE); /* kept */
    EXPECT_EQ(uart_available(), 0);
    uint8_t c = 0x55;
    EXPECT_FALSE(uart_read(&c));
    EXPECT_EQ(c, 0x55); /* untouched when empty */
}

TEST(bytes_come_out_in_arrival_order_and_each_is_acknowledged) {
    uart_init();
    sim_uart_receive('$');
    EXPECT_FALSE(USART1->SR & USART_SR_RXNE); /* the ISR acknowledged it */
    send("GPGGA");
    EXPECT_EQ(uart_available(), 6);
    const char *want = "$GPGGA";
    for (int i = 0; i < 6; i++) {
        uint8_t c = 0;
        EXPECT_TRUE(uart_read(&c));
        EXPECT_EQ(c, want[i]);
    }
    uint8_t c = 0;
    EXPECT_FALSE(uart_read(&c));
    EXPECT_EQ(uart_overruns(), 0); /* acknowledged in time: no false overruns */
    EXPECT_EQ(uart_dropped(), 0);
}

TEST(the_indices_wrap_around_over_a_long_stream) {
    uart_init();
    int next_in = 0, next_out = 0;
    for (int round = 0; round < 40; round++) {
        for (int k = 0; k < 7; k++) sim_uart_receive((uint8_t)(next_in++ & 0xFF));
        uint8_t c;
        while (uart_read(&c)) {
            EXPECT_EQ(c, next_out & 0xFF);
            next_out++;
        }
    }
    EXPECT_EQ(next_out, 280);
    EXPECT_EQ(uart_dropped(), 0);
}

TEST(a_full_buffer_drops_the_newest_bytes_and_counts_them) {
    uart_init();
    for (int i = 0; i < 20; i++) sim_uart_receive((uint8_t)i);
    EXPECT_EQ(uart_available(), RX_BUF_SIZE - 1);
    EXPECT_EQ(uart_dropped(), 5);
    EXPECT_FALSE(USART1->SR & USART_SR_RXNE); /* dropped bytes are still acknowledged */
    EXPECT_EQ(uart_overruns(), 0);
    for (int i = 0; i < (int)RX_BUF_SIZE - 1; i++) {
        uint8_t c = 0;
        EXPECT_TRUE(uart_read(&c));
        EXPECT_EQ(c, i); /* the oldest data survived */
    }
    sim_uart_receive('Z'); /* room again */
    uint8_t c = 0;
    EXPECT_TRUE(uart_read(&c));
    EXPECT_EQ(c, 'Z');
}

TEST(a_hardware_overrun_is_counted_and_cleared) {
    uart_init();
    __disable_irq();        /* someone held interrupts off too long... */
    sim_uart_receive('a');
    sim_uart_receive('b');  /* ...so 'b' overwrote 'a' in DR and the UART set ORE */
    __enable_irq();         /* the pending interrupt runs now */
    EXPECT_EQ(uart_overruns(), 1);
    EXPECT_FALSE(USART1->SR & USART_SR_ORE);
    EXPECT_FALSE(USART1->SR & USART_SR_RXNE);
    uint8_t c = 0;
    EXPECT_TRUE(uart_read(&c));
    EXPECT_EQ(c, 'b');
    EXPECT_FALSE(uart_read(&c));
    sim_uart_receive('c');
    EXPECT_EQ(uart_overruns(), 1); /* a normal byte is not an overrun */
}

TEST(a_spurious_interrupt_without_rxne_pushes_nothing) {
    uart_init();
    USART1->SR &= ~USART_SR_RXNE;
    USART1->DR = 'X';
    USART1_IRQHandler(); /* e.g. a TX interrupt sharing the vector */
    EXPECT_EQ(uart_available(), 0);
}

TEST(init_resets_the_buffer_and_the_counters) {
    uart_init();
    for (int i = 0; i < 18; i++) sim_uart_receive('x');
    uart_init();
    EXPECT_EQ(uart_available(), 0);
    EXPECT_EQ(uart_dropped(), 0);
    sim_uart_receive('k');
    uint8_t c = 0;
    EXPECT_TRUE(uart_read(&c));
    EXPECT_EQ(c, 'k');
}
