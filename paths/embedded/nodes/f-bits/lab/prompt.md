You're writing the GPIO driver for a new board. Before you touch any hardware, the driver needs a small toolkit of **pure functions** that work on a `uint32_t` register value. Each one takes a value and returns the new value, with no globals and no pointers, so it's easy to test.

```c
/* single bits: 0 <= bit <= 31 */
uint32_t reg_set_bit(uint32_t reg, unsigned bit);
uint32_t reg_clear_bit(uint32_t reg, unsigned bit);
uint32_t reg_toggle_bit(uint32_t reg, unsigned bit);
bool     reg_test_bit(uint32_t reg, unsigned bit);

/* multi-bit fields: 1 <= width <= 32 and pos + width <= 32 */
uint32_t reg_get_field(uint32_t reg, unsigned pos, unsigned width);
uint32_t reg_set_field(uint32_t reg, unsigned pos, unsigned width, uint32_t value);

/* GPIO MODER: pin n (0..15) owns bits 2n+1:2n */
uint32_t moder_set_pin_mode(uint32_t moder, unsigned pin, uint32_t mode);
uint32_t moder_get_pin_mode(uint32_t moder, unsigned pin);
```

The rules:

- Every function changes **only** the bits it's asked about. All other bits come back exactly as they went in.
- `reg_get_field` returns the field shifted down to bit 0.
- `reg_set_field` stores `value` in the field. If `value` doesn't fit in `width` bits, keep only its low `width` bits. It must never spill into the neighbouring bits.
- `width` can be 32, which means the whole register.
- The mode constants `PIN_MODE_INPUT`, `PIN_MODE_OUTPUT`, `PIN_MODE_ALT` and `PIN_MODE_ANALOG` (0 to 3) are in the starter.

```c
reg_set_bit(0, 31)                         // → 0x80000000
reg_get_field(0xABCD1234, 8, 8)            // → 0x12
reg_set_field(0xFFFFFFFF, 4, 4, 0x0)       // → 0xFFFFFF0F
moder_set_pin_mode(0xABFFFFFF, 5, PIN_MODE_OUTPUT)
                                           // → 0xABFFF7FF (PA13/PA14 untouched)
```

The tests run with UBSan, so a signed `1 << 31` or a `1u << 32` fails the test, just as it could fail on real hardware.
