#include <stddef.h>
#include <stdint.h>

static size_t varint_length(uint64_t value) {
    size_t len = 1;
    while (value >= 0x80) {
        value >>= 7;
        len++;
    }
    return len;
}

size_t varint_encode(uint64_t value, uint8_t *buf, size_t cap) {
    size_t len = varint_length(value);
    if (len > cap) {
        return 0;
    }
    for (size_t i = 0; i + 1 < len; i++) {
        buf[i] = (uint8_t)((value & 0x7F) | 0x80);   // 7 bits + "more follows"
        value >>= 7;
    }
    buf[len - 1] = (uint8_t)value;                   // last byte: high bit clear
    return len;
}
