#include <stdbool.h>
#include <stddef.h>

#define RING_CAP 4

typedef struct {
    int data[RING_CAP];
    size_t head;    // index of the oldest value (next to pop)
    size_t tail;    // index where the next push goes
    size_t count;   // how many values are stored
} RingBuffer;

void ring_init(RingBuffer *rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

bool ring_is_empty(const RingBuffer *rb) {
    return rb->count == 0;
}

bool ring_is_full(const RingBuffer *rb) {
    return rb->count == RING_CAP;
}

bool ring_push(RingBuffer *rb, int value) {
    if (ring_is_full(rb)) {
        return false;
    }
    rb->data[rb->tail] = value;
    rb->tail = (rb->tail + 1) % RING_CAP;
    rb->count++;
    return true;
}

bool ring_pop(RingBuffer *rb, int *out) {
    if (ring_is_empty(rb)) {
        return false;
    }
    *out = rb->data[rb->head];
    rb->head = (rb->head + 1) % RING_CAP;
    rb->count--;
    return true;
}
