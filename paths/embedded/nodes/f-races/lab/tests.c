#include <stdbool.h>
#include <stdint.h>
#include "mcu.h"

/* The race injector: fires one interrupt the first time the window opens. */
static void (*fire_once)(void);
static int window_calls;

void interrupt_window(void) {
    window_calls++;
    void (*f)(void) = fire_once;
    fire_once = 0;
    if (f) f();
}

static void tick_fires(void) { sim_systick(1); }
static void byte_arrives(void) { sim_uart_receive('$'); }

static void setup(void) {
    USART1->CR1 = USART_CR1_UE | USART_CR1_RE | USART_CR1_RXNEIE;
    window_calls = 0;
    fire_once = 0;
}

TEST(without_interrupts_everything_reads_back_correctly) {
    setup();
    ticks_lo = 0x00001234u;
    ticks_hi = 0x00000002u;
    EXPECT_UEQ(uptime_ticks(), 0x0000000200001234ull);
    pending_events = EVT_TICK | EVT_RX;
    events_clear(EVT_TICK);
    EXPECT_UEQ(pending_events, EVT_RX);
    EXPECT_UEQ(events_take(), EVT_RX);
    EXPECT_UEQ(pending_events, 0u);
    EXPECT_EQ(window_calls, 3); /* the windows are still there: the bugs were fixed, not hidden */
    EXPECT_TRUE(dts_irq_enabled());
}

TEST(uptime_never_tears_when_the_tick_carries_into_the_high_word) {
    setup();
    ticks_lo = 0xFFFFFFFFu; /* 49.7 days of uptime: the next tick carries */
    ticks_hi = 0;
    fire_once = tick_fires;
    uint64_t t = uptime_ticks();
    /* only a value the counter really had is acceptable; the torn read gives
       0x1_FFFFFFFF, which is 49 days in the future */
    EXPECT_TRUE(t == 0x00000000FFFFFFFFull || t == 0x0000000100000000ull);
    /* and the tick itself was not lost */
    EXPECT_UEQ(ticks_lo, 0u);
    EXPECT_UEQ(ticks_hi, 1u);
    EXPECT_TRUE(dts_irq_enabled());
}

TEST(events_take_never_loses_an_event_that_arrives_mid_swap) {
    setup();
    pending_events = EVT_TICK;
    fire_once = byte_arrives; /* EVT_RX lands between the read and the clear */
    uint32_t first = events_take();
    uint32_t second = events_take();
    EXPECT_UEQ(first | second, EVT_TICK | EVT_RX); /* every event delivered once... */
    EXPECT_UEQ(first & second, 0u);                 /* ...and only once */
    EXPECT_UEQ(pending_events, 0u);
    EXPECT_TRUE(dts_irq_enabled());
}

TEST(events_clear_keeps_bits_set_by_an_isr_in_the_middle) {
    setup();
    pending_events = EVT_TICK;
    fire_once = byte_arrives;
    events_clear(EVT_TICK);
    EXPECT_UEQ(pending_events, EVT_RX); /* the tick bit is gone, the new RX bit is not */
    EXPECT_TRUE(dts_irq_enabled());
}

TEST(a_tick_during_events_clear_is_not_wiped_out_either) {
    setup();
    pending_events = EVT_RX;
    fire_once = tick_fires;
    events_clear(EVT_RX);
    EXPECT_UEQ(pending_events, EVT_TICK);
    EXPECT_UEQ(ticks_lo, 1u);
}

TEST(nested_calls_leave_the_callers_critical_section_intact) {
    setup();
    ticks_lo = 41;
    pending_events = 0;
    __disable_irq(); /* the caller is already in its own critical section */
    fire_once = tick_fires;
    uint64_t t = uptime_ticks();
    EXPECT_FALSE(dts_irq_enabled()); /* you didn't switch them back on behind its back */
    EXPECT_UEQ(ticks_lo, 41u);       /* the tick is still pending, so it hasn't run yet */
    EXPECT_UEQ(events_take(), 0u);
    events_clear(EVT_RX);
    EXPECT_FALSE(dts_irq_enabled());
    __enable_irq();                  /* now the held-off tick runs */
    EXPECT_UEQ(t, 41u);
    EXPECT_UEQ(ticks_lo, 42u);
    EXPECT_UEQ(pending_events, EVT_TICK);
}

TEST(a_thousand_windows_and_not_one_lost_event) {
    setup();
    pending_events = 0;
    ticks_lo = 0xFFFFFF00u;
    ticks_hi = 7;
    unsigned ticks_seen = 0, rx_seen = 0;
    uint64_t last = uptime_ticks();
    for (int i = 0; i < 1000; i++) {
        fire_once = (i % 3 == 0) ? byte_arrives : tick_fires;
        uint32_t ev = events_take();
        ticks_seen += (ev & EVT_TICK) != 0;
        rx_seen += (ev & EVT_RX) != 0;
        fire_once = tick_fires;
        uint64_t now = uptime_ticks();
        EXPECT_TRUE(now >= last);         /* time never jumps backwards... */
        EXPECT_TRUE(now - last <= 3);     /* ...or 49 days forwards */
        last = now;
    }
    uint32_t ev = events_take();
    ticks_seen += (ev & EVT_TICK) != 0;
    rx_seen += (ev & EVT_RX) != 0;
    EXPECT_EQ(rx_seen, 334);
    EXPECT_TRUE(ticks_seen >= 666);
}
