#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

#define BQ_CAP 8

typedef struct {
    int buf[BQ_CAP];
    size_t head;              /* index of the oldest item */
    size_t count;             /* items in the queue */
    bool closed;
    pthread_mutex_t lock;     /* priority-inheritance mutex */
    pthread_cond_t not_empty; /* signalled when an item arrives */
    pthread_cond_t not_full;  /* signalled when a slot frees up */
} bqueue;

int bq_init(bqueue *q) {
    q->head = q->count = 0;
    q->closed = false;

    pthread_mutexattr_t attr;
    int rc = pthread_mutexattr_init(&attr);
    if (rc) return -rc;
    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    if (!rc) rc = pthread_mutex_init(&q->lock, &attr);
    pthread_mutexattr_destroy(&attr);   /* the mutex keeps its own copy of the settings */
    if (rc) return -rc;

    if ((rc = pthread_cond_init(&q->not_empty, NULL)) != 0) {
        pthread_mutex_destroy(&q->lock);
        return -rc;
    }
    if ((rc = pthread_cond_init(&q->not_full, NULL)) != 0) {
        pthread_cond_destroy(&q->not_empty);
        pthread_mutex_destroy(&q->lock);
        return -rc;
    }
    return 0;
}

void bq_destroy(bqueue *q) {
    pthread_cond_destroy(&q->not_full);
    pthread_cond_destroy(&q->not_empty);
    pthread_mutex_destroy(&q->lock);
}

int bq_push(bqueue *q, int value) {
    pthread_mutex_lock(&q->lock);
    while (q->count == BQ_CAP && !q->closed)    /* while, not if: wakeups can be spurious */
        pthread_cond_wait(&q->not_full, &q->lock);
    if (q->closed) {
        pthread_mutex_unlock(&q->lock);
        return -EPIPE;
    }
    q->buf[(q->head + q->count) % BQ_CAP] = value;
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
    return 0;
}

int bq_pop(bqueue *q, int *out) {
    pthread_mutex_lock(&q->lock);
    while (q->count == 0 && !q->closed)
        pthread_cond_wait(&q->not_empty, &q->lock);
    if (q->count == 0) {                        /* closed and drained */
        pthread_mutex_unlock(&q->lock);
        return -EPIPE;
    }
    *out = q->buf[q->head];
    q->head = (q->head + 1) % BQ_CAP;
    q->count--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->lock);
    return 0;
}

void bq_close(bqueue *q) {
    pthread_mutex_lock(&q->lock);
    q->closed = true;
    pthread_cond_broadcast(&q->not_empty);      /* wake every waiter so it can see `closed` */
    pthread_cond_broadcast(&q->not_full);
    pthread_mutex_unlock(&q->lock);
}

size_t bq_size(bqueue *q) {
    pthread_mutex_lock(&q->lock);
    size_t n = q->count;
    pthread_mutex_unlock(&q->lock);
    return n;
}
