#include "ringbuf.h"

bool rb_init(ringbuf_t *rb, uint8_t *storage, uint32_t size) {
    if (size < 2 || (size & (size - 1)) != 0) return false;
    rb->buf = storage;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->dropped = 0;
    return true;
}

bool rb_put(ringbuf_t *rb, uint8_t byte) {
    uint32_t head = rb->head;
    uint32_t next = (head + 1) & (rb->size - 1);
    if (next == rb->tail) {   /* full: drop the newest, never block */
        rb->dropped++;
        return false;
    }
    rb->buf[head] = byte;
    rb->head = next;          /* publish last */
    return true;
}

bool rb_get(ringbuf_t *rb, uint8_t *out) {
    uint32_t tail = rb->tail;
    if (tail == rb->head) return false;
    *out = rb->buf[tail];
    rb->tail = (tail + 1) & (rb->size - 1);   /* free the slot only after reading it */
    return true;
}

uint32_t rb_count(const ringbuf_t *rb) {
    return (rb->head - rb->tail) & (rb->size - 1);
}
