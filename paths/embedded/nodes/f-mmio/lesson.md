The firmware passed every test on the bench. QA signed off and the release build went to the factory. The first unit off the line printed one character on its debug UART and then froze.

The debug build was fine and the release build hung. The difference was one compiler flag, `-O2`, and one missing keyword:

```c
uint32_t *sr = (uint32_t *)0x40011000;   // USART1 status register
while (!(*sr & TXE)) { }                 // wait until the transmitter is free
```

The optimiser saw a loop that reads the same memory over and over while nothing in the loop writes to it, so it read the value **once**. If TXE was 0 at that moment, the loop spins forever on a stale copy in a CPU register, while the real hardware flag flipped to 1 microseconds later.

### Registers live at addresses

On a microcontroller, peripherals don't have a special instruction set. They sit **in the address space**. On an STM32F4, address `0x40020014` isn't RAM. It's wired to the output latches of GPIO port A, so writing a 1 there puts 3.3 V on a pin. This is **memory-mapped I/O (MMIO)**.

A peripheral is a block of registers at fixed offsets from a **base address**:

```
GPIOA base = 0x40020000      (simplified: the layout mcu.h uses;
                              the real F4 has OTYPER, OSPEEDR and PUPDR
                              in between, which is why its ODR is at +0x14)
  +0x00  MODER   pin modes
  +0x04  IDR     input levels (read-only)
  +0x08  ODR     output levels
  +0x0C  BSRR    bit set/reset (write-only)
```

You could poke raw addresses, but vendor headers (CMSIS) lay a **struct** over the block so the compiler does the offset maths:

```c
typedef struct {
    volatile uint32_t MODER, IDR, ODR, BSRR;
} GPIO_TypeDef;
#define GPIOA ((GPIO_TypeDef *)0x40020000u)

GPIOA->ODR |= 1u << 5;   // a load and a store at base + 0x08
```

`mcu.h` in your labs is built exactly like this. The only difference is that the "silicon" is a C simulator.

### Why `volatile`

`volatile` tells the compiler that this memory can change, or be observed, **behind your back**. Every read in the source must become a real load and every write a real store, in the order you wrote them relative to other `volatile` accesses, with nothing cached, merged or deleted.

Without it, the compiler is allowed to:
- **cache reads**, so your polling loop spins forever (the story above)
- **delete "dead" writes**, so `REG = 1; REG = 0;` becomes just `REG = 0` and your strobe pulse vanishes
- **reorder** independent register accesses

`volatile` is **not** a lock. It doesn't make anything atomic and it doesn't stop interrupts. It only forces the accesses to happen.

### Read-modify-write, and its trap

`GPIOA->ODR |= 1u << 5` is really three steps: **load** ODR, **OR** in the bit, **store** it back. If an interrupt fires between the load and the store and toggles PA3 in ODR, your store writes back the *old* PA3. The ISR's change is lost. (You'll fight this bug for real in *Races & critical sections*.)

That's why STM32 has **BSRR**, the bit set/reset register. It's write-only and has no memory of its own:

```
BSRR bits 0-15:  write 1 → set that pin's output to 1
BSRR bits 16-31: write 1 → reset that pin's output to 0
writing 0 anywhere: no effect
```

```c
GPIOA->BSRR = 1u << 5;          // PA5 high, one store, nothing read
GPIOA->BSRR = 1u << (5 + 16);   // PA5 low
```

A single store with no read can't lose anyone else's update. The silicon does the "modify" part atomically. Note that it's `=`, not `|=`: on real hardware BSRR reads back as 0, so `|=` would only waste a load.

### Worked example: init, drive, read

```c
void board_init(void) {
    uint32_t m = GPIOA->MODER;          // RMW of a config register at init:
    m &= ~(3u << (5 * 2));              // fine, because no ISR touches it yet
    m |=  (1u << (5 * 2));
    GPIOA->MODER = m;
}
bool button_pressed(void) {
    return (GPIOC->IDR & (1u << 13)) == 0;   // active-low: pressed = 0 V
}
```

Buttons are often **active-low**. A pull-up resistor holds the pin at 1, and pressing the button shorts it to ground. "Pressed" reads as **0**, which surprises everyone the first time.

### Gotchas

- **`|=` on a write-only register.** BSRR, interrupt-clear registers and many FIFOs are write-only or "write 1 to clear". Reading them returns junk or zero, or worse, the read itself has side effects. Some status registers clear a flag *when you read them*, so a debugger watch window can "steal" your interrupt.
- **Volatile everywhere isn't a fix.** Sprinkling `volatile` on ordinary variables to "make a race go away" just hides the race.
- **Pointer arithmetic is in elements, not bytes.** `(volatile uint32_t *)base + 0x08` lands at byte offset 0x20. Add the offset to the integer address *first*, then cast.
- **Clocks.** On real chips, a peripheral's registers ignore writes until you enable its clock (`RCC->AHB1ENR`). "My GPIO doesn't work" is a missing clock enable half the time.

### In the wild

- Every Cortex-M vendor ships CMSIS headers like `stm32f4xx.h`: thousands of lines of `volatile` structs and `_Pos`/`_Msk` macros.
- Linux drivers use `readl()`/`writel()` on `ioremap()`ed addresses. Those wrappers exist to add `volatile` *and* memory barriers.
- Rust's embedded HAL generates register structs from the vendor's SVD file, with `read()`, `write()` and `modify()` closures. That's read-modify-write written into the API.
- BSRR-style set/clear registers are everywhere: NXP's `PSOR`/`PCOR`, Nordic's `OUTSET`/`OUTCLR` and RP2040's atomic `SET`/`CLR`/`XOR` aliases all exist for the same reason.

In the lab you'll bring up an LED and a button on the simulated MCU: map raw addresses to registers, configure the mode field, drive the pin through BSRR and read the button through IDR.
