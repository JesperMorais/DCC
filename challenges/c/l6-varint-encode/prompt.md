Telemetry packets are small, and most numbers in them are small too, so the protocol uses **varints** (unsigned LEB128, the encoding in Protocol Buffers and WebAssembly) instead of fixed 8-byte integers:

- Split the value into 7-bit groups, **least significant group first**.
- Each group becomes one byte. Set the high bit (`0x80`) on every byte **except the last**, to mean "more bytes follow".

```c
size_t varint_encode(uint64_t value, uint8_t *buf, size_t cap);
```

- Write the encoding of `value` into `buf` and return the number of bytes written (1 to 10).
- `cap` is the size of `buf`. If the encoding doesn't fit, return `0` and **write nothing**. Never write past `buf[cap - 1]`.

| value | bytes |
|---|---|
| `0` | `00` |
| `127` | `7F` |
| `128` | `80 01` |
| `300` | `AC 02` |
| `UINT64_MAX` | `FF FF FF FF FF FF FF FF FF 01` |

For example, 300 is `0b10_0101100`: the low 7 bits `0101100` (`0x2C`) come first with the continuation bit set (`0xAC`), then the remaining `10` (`0x02`).
