**Bug report from the update service**

> Devices fetch firmware as a stream of TLV records. Small records parse fine, but some chunks come back as `TLV_TRUNCATED` even though the whole download is in the buffer. A 300-byte chunk worked yesterday, a 200-byte chunk didn't today. We can't see a pattern.

Each record is a **type** byte, a **length** as two bytes, big-endian (high byte first), then `length` bytes of value. The bytes arrive from `recv` into a `char` buffer.

- `tlv_next` reads the record at `r->pos` into `*out` (`value` points into the buffer, not a copy), advances past it and returns `TLV_OK`.
- At the end of the buffer it returns `TLV_END`.
- If the header or value runs past the end of the buffer it returns `TLV_TRUNCATED` and doesn't advance.
- `tlv_find` scans from the start and copies the first record with the given type into `*out`, returning `false` if there is none.

```
01 00 04 'a' 'b' 'c' 'd'   → type 1, length 4, value "abcd"
07 00 00                   → type 7, length 0
05 01 2C <300 bytes>       → type 5, length 300
```

**Reproduction**

```c
char buf[256] = {3, 0x00, (char)0xC8};   // type 3, length 200, then 200 value bytes
TlvReader r;
tlv_reader_init(&r, buf, 203);
Tlv t;
tlv_next(&r, &t);   // returns TLV_TRUNCATED, expected TLV_OK with t.length == 200
```
