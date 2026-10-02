A GPS module streams NMEA sentences into **USART1** at 115200 baud. The main loop is busy drawing a map and can't poll fast enough, so the bytes have to be caught by the RX interrupt and parked in a ring buffer until the main loop gets to them.

Using `mcu.h`, write:

```c
#define RX_BUF_SIZE 16u   /* a power of two; the buffer holds RX_BUF_SIZE - 1 = 15 bytes */

void     uart_init(void);
void     USART1_IRQHandler(void);   /* the ISR: the producer */
bool     uart_read(uint8_t *out);   /* the main loop: the consumer */
size_t   uart_available(void);
uint32_t uart_dropped(void);
uint32_t uart_overruns(void);
```

**`uart_init`**
- Empty the buffer and zero both counters.
- Enable the USART with its receiver and RX interrupt: set `USART_CR1_UE`, `USART_CR1_RE` and `USART_CR1_RXNEIE` in `CR1`, and **keep any other CR1 bits** that are already set.

**`USART1_IRQHandler`** (the tests call `sim_uart_receive(byte)`, which plays the hardware and invokes your ISR)
- If `RXNE` isn't set in `SR`, it isn't a receive interrupt. Do nothing.
- Otherwise read the byte from `DR`, then **acknowledge**: clear `RXNE` in `SR`. The simulated USART doesn't clear it on a `DR` read, so write the 0 yourself: `USART1->SR &= ~...`.
- If `ORE` is set, the hardware lost a byte before you got here. Count one overrun and clear `ORE` too.
- Push the byte into the ring. If the ring is **full**, drop the *new* byte and count it in `uart_dropped`. Never overwrite unread data, and never wait.

**The ring buffer**
- It's a single-producer/single-consumer ring with **no shared count**. Only the ISR writes `head` and only `uart_read` writes `tail`. One slot always stays empty, so the buffer holds 15 bytes.
- `uart_read` pops the oldest byte into `*out` and returns `true`. When the buffer is empty it returns `false` and leaves `*out` alone.
- `uart_available` returns how many bytes are waiting.

```c
uart_init();
sim_uart_receive('$');  sim_uart_receive('G');
uart_available();       // → 2
uint8_t c;
uart_read(&c);          // → true, c == '$'
```

One test switches interrupts **off** with `__disable_irq()` while two bytes arrive. The UART can only hold one, so you'll see a real ORE.
