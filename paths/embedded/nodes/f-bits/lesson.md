It's 2 a.m. and the new board was working an hour ago. You added one line to blink an LED on PA5:

```c
GPIOA->MODER = 1 << 10;   // "make PA5 an output"
```

The LED blinks. Then you hit "Debug" and the IDE says **"No target connected"**. Reflashing fails too. The board looks bricked.

It isn't, but you've learned the first lesson of firmware the hard way. `MODER` holds the mode of **all 16 pins** of port A. Your `=` wrote zeros into every other pin's field, and on an STM32, PA13 and PA14 are the SWD debug pins. You just turned your debugger's wires into plain inputs. (The fix is "connect under reset", and a lot of swearing.)

The right line changes **only** the two bits that belong to PA5. This node is about writing that line without thinking twice.

### Registers are just numbers with a map

A peripheral register is a 32-bit number in which each bit, or small group of bits, means something. The reference manual gives you the map:

```
MODER (GPIO port mode register)
 31 30 29 28            11 10  9  8  7  6  5  4  3  2  1  0
[ pin15 ][ pin14 ] ... [ pin5 ][ pin4 ][ pin3 ][ pin2 ][ pin1 ][ pin0 ]
 00 = input, 01 = output, 10 = alternate function, 11 = analog
```

Pin `n` owns bits `2n` and `2n+1`. That makes it a **field**: several adjacent bits read and written as one small number.

Because the hardware cares about exact widths, firmware uses the fixed-width types from `<stdint.h>`: `uint8_t`, `uint16_t` and `uint32_t`. An `int` might be 16 bits on one chip and 32 on another. A `uint32_t` is 32 bits everywhere.

### The four moves on a single bit

```c
reg |=  (1u << n);          // set bit n
reg &= ~(1u << n);          // clear bit n
reg ^=  (1u << n);          // toggle bit n
if (reg & (1u << n)) {...}  // test bit n
```

`1u << n` is a **mask** with exactly one bit set. OR puts that bit on, AND with the inverted mask takes it off, and XOR flips it. Every other bit passes through untouched, and that's the whole point.

### Fields: extract and insert

To **read** a field, shift it down to bit 0 and mask off its neighbours:

```c
uint32_t mask = (1u << width) - 1u;      // width 2 → 0b11
uint32_t mode = (reg >> pos) & mask;
```

To **write** a field, clear it first, then OR in the new value, shifted into place:

```c
reg = (reg & ~(mask << pos)) | ((value & mask) << pos);
```

There are two steps there, and forgetting either one is a classic bug:
- If you skip the clear, `01` OR `10` gives `11`: you asked for "alternate function" and got "analog".
- If you skip `value & mask`, a value that's too big for the field spills into the next pin's bits.

### Worked example: PA5 as an output, done right

```c
uint32_t moder = GPIOA->MODER;            // 0xABFF_FFFF after reset on many STM32s
moder &= ~(3u << (5 * 2));                // clear bits 11:10
moder |=  (1u << (5 * 2));                // 01 = output
GPIOA->MODER = moder;                     // PA13/PA14 still say "10", so SWD lives
```

Read, modify, write. Only two bits changed, and your debugger stays connected.

### Gotchas

**`1 << 31` is undefined behaviour.** The literal `1` is a signed `int`, and shifting a 1 into the sign bit can't be represented. Most compilers happen to give you `0x80000000`, but the C standard says anything may happen, and UBSan (which runs on your labs) stops the program dead. Always shift **unsigned** values: `1u << 31` or `UINT32_C(1) << 31`.

**Shifting by the full width is also UB.** `1u << 32` on a 32-bit type isn't 0. It's undefined, and on ARM and x86 the hardware often uses only the low 5 bits of the shift count, so it quietly comes out as `1u << 0`. A "mask for a 32-bit-wide field" needs its own special case: `width >= 32 ? 0xFFFFFFFFu : (1u << width) - 1u`.

**`~` promotes too.** `~(uint8_t)0x0F` is the `int` `0xFFFFFFF0`, not `0xF0`. Do your bit maths in `uint32_t` and cast at the end.

**Operator precedence.** `reg & 1u << n == 0` parses as `reg & ((1u << n) == 0)`. Use parentheses, every time.

### In the wild

- **CMSIS vendor headers** define every field as a `_Pos` and `_Msk` pair, for example `GPIO_MODER_MODE5_Pos` and `GPIO_MODER_MODE5_Msk`. Real drivers are full of `(reg & ~X_Msk) | (val << X_Pos)`, which is exactly the insert you'll write in the lab.
- **Linux** has `FIELD_GET()` / `FIELD_PREP()` and `GENMASK(h, l)` in `<linux/bitfield.h>`. They exist because people kept getting this wrong.
- **Zephyr** has `BIT(n)`, `GENMASK`, `FIELD_GET` and `WRITE_BIT`.
- **The SWD lockout** is so common that every STM32 forum has a pinned "connect under reset" thread. Now you know where it comes from.

In the lab you'll build the helper toolkit a GPIO driver sits on, and the tests check it at bit 31, where signed-shift bugs hide.
