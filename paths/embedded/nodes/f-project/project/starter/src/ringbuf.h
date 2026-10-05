/* Milestone 1: a lock-free single-producer, single-consumer byte ring. */
#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    volatile uint8_t *buf;      /* storage, owned by the caller */
    uint32_t size;              /* number of slots, a power of two */
    volatile uint32_t head;     /* next slot to write. Written only by the producer */
    volatile uint32_t tail;     /* next slot to read. Written only by the consumer */
    volatile uint32_t dropped;  /* bytes refused because the ring was full. Producer only */
} ringbuf_t;

/* Sets rb up over storage[size]: empty, nothing dropped. Returns false (and
 * leaves rb alone) unless size is a power of two and at least 2. */
bool rb_init(ringbuf_t *rb, uint8_t *storage, uint32_t size);

/* Producer side. Appends byte. When the ring is full, drops byte (the newest),
 * counts it in rb->dropped and returns false. Holds at most size - 1 bytes. */
bool rb_put(ringbuf_t *rb, uint8_t byte);

/* Consumer side. Takes the oldest byte into *out. Returns false when empty. */
bool rb_get(ringbuf_t *rb, uint8_t *out);

/* How many bytes are waiting. */
uint32_t rb_count(const ringbuf_t *rb);

#endif
