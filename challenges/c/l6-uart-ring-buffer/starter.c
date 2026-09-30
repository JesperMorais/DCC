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
    (void)rb;
}

bool ring_push(RingBuffer *rb, int value) {
    (void)rb;
    (void)value;
    return false;
}

bool ring_pop(RingBuffer *rb, int *out) {
    (void)rb;
    (void)out;
    return false;
}

bool ring_is_empty(const RingBuffer *rb) {
    (void)rb;
    return true;
}

bool ring_is_full(const RingBuffer *rb) {
    (void)rb;
    return false;
}
