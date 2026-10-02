The firmware in the starter shipped last month. The field reports are strange: about once a day a unit logs an uptime **49 days in the future**, and every now and then a received UART byte is never processed. Nobody can reproduce either one on the bench.

The interrupt handlers are fine:

```c
void SysTick_Handler(void)   { if (++ticks_lo == 0) ticks_hi++;  pending_events |= EVT_TICK; }
void USART1_IRQHandler(void) { /* ack the byte */                pending_events |= EVT_RX;   }
```

The bugs are in the three main-loop functions that share data with them:

```c
uint64_t uptime_ticks(void);          /* the 64-bit uptime, stored as two 32-bit halves */
uint32_t events_take(void);           /* return all pending event bits and clear them */
void     events_clear(uint32_t mask); /* clear only the bits in mask, keep the rest */
```

Each one reads shared data, calls **`interrupt_window()`**, and then writes or reads again. On real silicon an interrupt can land between *any* two instructions. The tests define `interrupt_window()` to fire a real SysTick or UART interrupt at that exact point, every run, so the "once a day" bug happens every time.

**Fix the races:**

- **Don't remove or move the `interrupt_window()` calls.** They stand in for the instructions an interrupt can land between, and the tests check that they're still called.
- Make each read…window…write sequence **atomic** with respect to interrupts, using `__disable_irq()` / `__enable_irq()` from `mcu.h`. The interrupt isn't lost: the simulated NVIC holds it pending and runs it as soon as you re-enable.
- `uptime_ticks` must return a value the counter really had: either the value just before the tick or just after it, never a mix of the two halves.
- No event may be lost. A bit that an ISR sets during `events_take` or `events_clear` must still be pending afterwards, unless `events_take` returned it.
- **Leave interrupts the way you found them.** If they were on, they must be on again when you return. If the caller had already disabled them, they must still be off. (`dts_irq_enabled()` tells you the current state, like `__get_PRIMASK()` in CMSIS.)
- Keep the critical sections short: only the shared accesses (and the window) go inside.

Keep the variable names (`ticks_lo`, `ticks_hi`, `pending_events`). The tests set them directly.
