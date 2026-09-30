### Numbers are rows of bits

`uint32_t` (from `<stdint.h>`) is exactly 32 bits with no sign. Writing it in hex helps, because each hex digit is exactly 4 bits: `0xA5` is `1010 0101`.

| Operator | Bit-by-bit rule | Typical use |
|---|---|---|
| `a & b` | 1 if both are 1 | keep only some bits (masking) |
| `a \| b` | 1 if either is 1 | turn bits on |
| `a ^ b` | 1 if they differ | flip bits |
| `~a` | flips every bit | build "everything except" masks |
| `a << k`, `a >> k` | shift left/right by k | move bits into position |

A **mask** is a number with 1s exactly where you want to act:

```c
uint32_t low_nibble = 0x0Fu;
uint32_t status = reg & low_nibble;   // keep bits 0–3, zero the rest
reg = reg & ~low_nibble;              // clear bits 0–3, keep the rest
```

### Signed shifts are a trap

The literal `1` is a signed `int`. `1 << 31` would need the sign bit, and that's **undefined behavior** in C. UBSan stops the program. Add a `u` suffix (`1u`) and the arithmetic is unsigned, where every shift from 0 to 31 is well defined. Shifting by 32 or more is undefined even for unsigned values.

### A classic trick

Subtracting 1 from a number flips its lowest 1-bit to 0 and every 0 below it to 1: `0b1011000 - 1 = 0b1010111`. Combine that with `&` and see what you get.
