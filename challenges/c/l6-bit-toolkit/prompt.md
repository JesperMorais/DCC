Firmware talks to hardware through 32-bit registers, where every bit means something. Write four helpers that driver code keeps reaching for. Bit `n` is the bit worth `2ⁿ`, so bit 0 is the lowest and bit 31 the highest. `n` is always in `0..31`.

```c
bool     bit_is_set(uint32_t x, unsigned n);   // is bit n 1?
unsigned count_set_bits(uint32_t x);           // how many bits are 1
bool     is_power_of_two(uint32_t x);          // exactly one bit set? (0 is not)
uint32_t reverse_bits(uint32_t x);             // bit 0 ↔ bit 31, bit 1 ↔ bit 30, …
```

```c
bit_is_set(0x10, 4)         // → true
bit_is_set(0x80000000, 31)  // → true
count_set_bits(0xF0F0)      // → 8
is_power_of_two(64)         // → true
reverse_bits(0x00000001)    // → 0x80000000
```

Don't use compiler builtins such as `__builtin_popcount`. The point is to do it with shifts and masks.
