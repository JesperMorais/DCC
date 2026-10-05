July 20, 1969, a few minutes before the first Moon landing. The Apollo Guidance Computer flashes **1202**, then **1201**: *executive overflow*. A radar switch had been left in a position that made the rendezvous radar's interface hammer the computer with spurious counter-update requests (hardware "cycle steals", tiny interrupts handled in hardware), and they were eating about 13% of the CPU. The landing software couldn't finish its work in time.

What saved the landing was good real-time design. The computer shed low-priority jobs, restarted and kept flying, and Armstrong landed. Interrupts are the most powerful tool in firmware and the easiest to abuse.

### Polling vs interrupts

**Polling** means asking over and over: "is there a byte yet? is there a byte yet?"

```c
for (;;) {
    if (USART1->SR & USART_SR_RXNE) handle(USART1->DR);
    update_display();          // 20 ms... and bytes pile up
}
```

It's simple, but you only notice an event when you get round to checking. At 115200 baud a byte arrives every **87 µs**, and the UART holds exactly *one*. If `update_display()` takes 20 ms, you've lost about 230 bytes.

With an **interrupt**, the hardware taps the CPU on the shoulder. It finishes the current instruction, saves registers, jumps to your **ISR** (interrupt service routine) and comes back as if nothing happened:

```
main:   ──display──display──┐    ┌──display──
                            │ISR │
UART:          byte! ───────┘    └ (2 µs later)
```

The time from the event to the first line of your ISR is the **interrupt latency**. On a Cortex-M it's about 12 cycles, as long as nobody has interrupts disabled.

### The rules of ISR club

1. **Keep it short.** While your ISR runs, equal and lower-priority interrupts wait. A 1 ms ISR adds 1 ms of latency to everything below it.
2. **Never block.** No `delay()`, no waiting on a flag, no mutex, no `printf` to a slow UART. You'd wait for something that may need the CPU you're holding.
3. **Acknowledge the source.** Clear the flag that caused the interrupt (or read the data register), or the ISR fires again the instant it returns. That's an *interrupt storm*.
4. **Share data carefully.** The main loop can be interrupted *between any two instructions*. More on that in the next node.

So the ISR's job is to **grab the data and get out**, and the real work happens later in the main loop or a task. The standard way to hand the data over is a **ring buffer**.

### The lock-free SPSC ring buffer

There's one **producer** (the ISR) and one **consumer** (the main loop):

```
buf: [ . ][ h ][ e ][ l ][ . ][ . ][ . ][ . ]
            ▲tail             ▲head
     consumer reads at tail   producer writes at head
```

- The ISR writes `buf[head]`, then advances `head`.
- The main loop reads `buf[tail]`, then advances `tail`.
- `head == tail` means **empty**. "Full" is `next(head) == tail`, so one slot always stays empty. That way full and empty look different without a shared count.

Why no lock? **Each index is written by only one side.** The ISR never writes `tail`, and `main` never writes `head`. A shared `count` would be written by both (`count++` in the ISR, `count--` in main), which is a race. The indices are `volatile` so neither side caches the other's updates, and the ISR stores the byte *before* publishing the new `head`, so the consumer never sees a slot that isn't filled yet.

### Worked example: a UART RX ISR

```c
void USART1_IRQHandler(void) {
    uint32_t sr = USART1->SR;
    if (!(sr & USART_SR_RXNE)) return;            // not ours
    uint8_t byte = (uint8_t)USART1->DR;           // grab it now
    /* ...acknowledge flags, check ORE... */
    uint32_t next = (head + 1) & (SIZE - 1);
    if (next == tail) { dropped++; return; }      // full: drop, never block
    buf[head] = byte;
    head = next;                                  // publish last
}
```

**Overflow policy is a design decision.** Here we drop the *newest* byte and count it. The alternative, overwriting the oldest, needs the producer to move `tail`, which breaks the one-writer-per-index rule. Either way, **count it**, because a silent drop is a bug report with no clues.

**ORE** (overrun error) is the hardware's version of the same thing. A second byte arrived before anyone read the first, because interrupts were masked too long, so the UART lost one. The ring buffer can't fix that, but your ISR can notice it, count it and clear the flag.

### Gotchas

- **Power-of-two sizes** let you wrap with `& (SIZE - 1)` instead of `%`. Division can take dozens of cycles on small cores.
- **Forgetting to acknowledge** an interrupt leaves you stuck in it forever, and the main loop never runs again.
- **`volatile` on the indices** is required, and it's also *all* you get. It works here only because of the single-writer design.
- **`volatile` only orders `volatile` accesses.** The compiler may still move a plain `buf[]` access across an index update (say, read `buf[tail]` *after* storing the new `tail`, when the ISR may already be refilling that slot). On a single core the ISR runs to completion, so its own store order can't be observed half-done, but the consumer side can bite. Production rings make `buf` `volatile` too, or use C11 atomics (release on publish, acquire on read). With two cores or DMA you also need real memory barriers.
- **Overrun is not a buffer-full condition.** ORE means the hardware lost a byte *before* the ISR ran. Overflow means you lost one *after*. They have different causes and different fixes.
- **Know how your part clears its flags.** On a real STM32F4, ORE is read-only: reading `SR` and then `DR` clears it. Status bits that *are* cleared by writing 0 (`rc_w0`, like TC) should be cleared with a plain write such as `USART1->SR = ~USART_SR_TC`, not `&= ~`. A read-modify-write can clear a flag that the hardware set between your read and your write. The simulator in the lab accepts the explicit clear; the reference manual is the authority on real silicon.

### In the wild

- Every serial driver: Arduino's `HardwareSerial` (a 64-byte ring), Zephyr's `uart_irq_*` + `ring_buf`, Linux's `tty` flip buffers.
- Audio and sensor DMA uses the same idea at scale: a DMA "half-transfer" interrupt says one half of a buffer is ready while the other half fills.
- Lock-free SPSC queues show up far beyond microcontrollers, too: the LMAX Disruptor, Linux `kfifo` and game-engine audio threads.

In the lab you'll write the UART RX ISR on the simulated MCU, the ring buffer the main loop drains, and the bookkeeping for both overflow and ORE.
