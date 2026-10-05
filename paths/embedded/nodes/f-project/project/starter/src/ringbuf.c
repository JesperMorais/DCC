#include "ringbuf.h"

bool rb_init(ringbuf_t *rb, uint8_t *storage, uint32_t size) {
    (void)rb, (void)storage, (void)size;
    return false;   /* TODO(m1) */
}

bool rb_put(ringbuf_t *rb, uint8_t byte) {
    (void)rb, (void)byte;
    return false;   /* TODO(m1) */
}

bool rb_get(ringbuf_t *rb, uint8_t *out) {
    (void)rb, (void)out;
    return false;   /* TODO(m1) */
}

uint32_t rb_count(const ringbuf_t *rb) {
    (void)rb;
    return 0;       /* TODO(m1) */
}
