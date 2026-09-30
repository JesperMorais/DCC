### Variable-length integers

A fixed-width `uint64_t` always costs 8 bytes, even for the number 3. A varint spends bytes only as needed: one byte for values under 128, two under 16,384, and so on. Every byte has two jobs:

```
bit:   7        6 5 4 3 2 1 0
       more?    ─── 7 payload bits ───
```

Decoding reads bytes until one has its top bit clear. That's why the encoder must mark every byte except the last.

### Extracting bit groups

A mask picks out the low bits, and a right shift moves the next group into place:

```c
uint32_t rgb = 0x12AB34;
uint8_t blue  = rgb & 0xFF;          // 0x34
uint8_t green = (rgb >> 8) & 0xFF;   // 0xAB
```

Converting a wider integer to `uint8_t` keeps only the low 8 bits. An explicit cast, `(uint8_t)x`, says you meant that.

### Caller-provided buffers

Much systems C avoids `malloc` and has the **caller** supply the memory, together with its size. The function promises never to write beyond that size and reports how much it used, or that it didn't fit. `snprintf` works this way. It's also why the size check has to happen *before* writing: a partial write into a too-small buffer is a buffer overflow.
