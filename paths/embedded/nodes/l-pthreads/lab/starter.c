#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

#define BQ_CAP 8

/* Keep these field names: the tests poke at the condition variables to fake spurious wakeups. */
typedef struct {
    int buf[BQ_CAP];
    size_t head;              /* index of the oldest item */
    size_t count;             /* items in the queue */
    bool closed;
    pthread_mutex_t lock;     /* must be a priority-inheritance mutex */
    pthread_cond_t not_empty; /* signalled when an item arrives */
    pthread_cond_t not_full;  /* signalled when a slot frees up */
} bqueue;

int bq_init(bqueue *q) {
    q->head = q->count = 0;
    q->closed = false;
    pthread_mutex_init(&q->lock, NULL);   /* TODO: PTHREAD_PRIO_INHERIT */
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
    return 0;
}

void bq_destroy(bqueue *q) {
    pthread_cond_destroy(&q->not_full);
    pthread_cond_destroy(&q->not_empty);
    pthread_mutex_destroy(&q->lock);
}

/* Block while full. 0 on success, -EPIPE once the queue is closed. */
int bq_push(bqueue *q, int value) {
    (void)q;
    (void)value;
    return -ENOSYS;
}

/* Block while empty. 0 on success, -EPIPE when the queue is closed AND empty. */
int bq_pop(bqueue *q, int *out) {
    (void)q;
    (void)out;
    return -ENOSYS;
}

/* Wake everyone up: pushes fail from now on, pops drain what's left. */
void bq_close(bqueue *q) {
    (void)q;
}

size_t bq_size(bqueue *q) {
    return q->count;
}
