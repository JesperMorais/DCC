#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    const char *data;
    size_t size;
    size_t pos;
} TlvReader;

typedef struct {
    uint8_t type;
    uint16_t length;
    const char *value;
} Tlv;

typedef enum {
    TLV_OK,
    TLV_END,
    TLV_TRUNCATED,
} TlvStatus;

void tlv_reader_init(TlvReader *r, const char *data, size_t size) {
    r->data = data;
    r->size = size;
    r->pos = 0;
}

TlvStatus tlv_next(TlvReader *r, Tlv *out) {
    size_t remaining = r->size - r->pos;
    if (remaining == 0) {
        return TLV_END;
    }
    if (remaining < 3) {
        return TLV_TRUNCATED;
    }
    const char *p = r->data + r->pos;
    uint16_t length = (uint16_t)((unsigned char)p[1] << 8 | (unsigned char)p[2]);
    if (remaining - 3 < length) {
        return TLV_TRUNCATED;
    }
    out->type = (uint8_t)p[0];
    out->length = length;
    out->value = p + 3;
    r->pos += 3 + (size_t)length;
    return TLV_OK;
}

bool tlv_find(const char *data, size_t size, uint8_t type, Tlv *out) {
    TlvReader r;
    tlv_reader_init(&r, data, size);
    Tlv t;
    while (tlv_next(&r, &t) == TLV_OK) {
        if (t.type == type) {
            *out = t;
            return true;
        }
    }
    return false;
}
