### Bytes, `char` and integer promotion

Network and file code deals in **bytes**, values 0–255. C has three character types to hold them, and they are not the same:

| type | range |
|---|---|
| `unsigned char` / `uint8_t` | 0 to 255 |
| `signed char` | −128 to 127 |
| `char` | either of the above, chosen by the platform (signed on x86 and most ARM Linux) |

Before almost any arithmetic, C **promotes** small types to `int`. An `unsigned char` holding `0xC8` becomes `200`. A signed `char` holding the same bit pattern becomes `-56`, which as an `int` is `0xFFFFFFC8`: the sign bit is copied into every higher bit.

### Assembling multi-byte numbers

A big-endian 16-bit number is "high byte times 256 plus low byte", usually written with a shift and an OR:

```c
const uint8_t b[] = {0x01, 0x2C};
uint16_t n = (uint16_t)(b[0] << 8 | b[1]);   // 0x012C = 300
```

This works because both bytes promote to small, non-negative `int`s. The same expression on bytes that might be negative after promotion does something quite different.

### Debugging: print the bytes in hex

When a parser misreads binary data, print the raw bytes and the intermediate values in hex (`printf("%02x %02x -> %x\n", ...)`). Decimal hides bit patterns; hex makes "these failing values all have the top bit set" jump out.
