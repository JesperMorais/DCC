### Bitmaps and the types of your constants

A bitmap packs one yes/no flag per item into the bits of an integer. Setting, clearing and testing bit `k` all use a mask with only that bit set:

```c
uint32_t word = 0;
word |=  mask;           // set
word &= ~mask;           // clear
bool on = word & mask;   // test
```

The catch is the **type** of the expression that builds the mask. An integer literal like `1` or `0x10` is an `int`, and arithmetic happens in the type of its operands, not in the type of the variable the result is stored in:

```c
uint64_t big = 1 << 40;            // computed as int: undefined
uint64_t ok  = UINT64_C(1) << 40;  // computed as uint64_t
```

For shifts, the rules are strict: shifting by at least the width of the type is undefined, and for signed types so is any shift whose result doesn't fit. Undefined behaviour often "works" on one compiler and breaks under another or at a higher optimisation level, so code can run for months before it bites.

### Debugging: test the boundaries of the representation

Integer bugs hide at the edges: bit 0, the top bit, the sign bit, the last element of the last word. When a bitset works for "small" indices, test the highest bit of each word explicitly. Sanitizers (`-fsanitize=undefined`) turn silent UB into a report at the exact expression.
