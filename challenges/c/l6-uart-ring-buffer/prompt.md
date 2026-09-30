A serial port's interrupt handler stores incoming bytes, and the main loop consumes them later. There's no `malloc` on this microcontroller, so the queue is a fixed-size **ring buffer**:

```c
#define RING_CAP 4

typedef struct {
    int data[RING_CAP];
    size_t head;    // index of the oldest value (next to pop)
    size_t tail;    // index where the next push goes
    size_t count;   // how many values are stored
} RingBuffer;

void ring_init(RingBuffer *rb);
bool ring_push(RingBuffer *rb, int value);
bool ring_pop(RingBuffer *rb, int *out);
bool ring_is_empty(const RingBuffer *rb);
bool ring_is_full(const RingBuffer *rb);
```

- `ring_init` makes the buffer empty.
- `ring_push` appends `value` and returns `true`. If the buffer is full it returns `false` and changes nothing. It never overwrites unread data.
- `ring_pop` removes the **oldest** value into `*out` and returns `true`. If the buffer is empty it returns `false` and leaves `*out` unchanged.
- Values come out in the order they went in (FIFO), including after the indices wrap past the end of `data`.

```c
RingBuffer rb;
ring_init(&rb);
ring_push(&rb, 1); ring_push(&rb, 2);
int v;
ring_pop(&rb, &v);   // → true, v = 1
ring_pop(&rb, &v);   // → true, v = 2
ring_pop(&rb, &v);   // → false, v still 2
```

The struct and `RING_CAP` are in the starter. Keep using `RING_CAP` rather than a literal 4.
