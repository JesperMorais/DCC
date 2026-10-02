#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static void sleep_ms(long ms) {
    struct timespec t = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&t, NULL);
}

TEST(the_mutex_uses_priority_inheritance) {
    bqueue q;
    EXPECT_EQ(bq_init(&q), 0);
    /* glibc keeps the protocol inside the mutex itself. Peek at it the way a debugger would:
     * 0x20 is PTHREAD_MUTEX_PRIO_INHERIT_NP. */
    EXPECT_TRUE((q.lock.__data.__kind & 0x20) != 0);
    bq_destroy(&q);
}

TEST(single_thread_fifo_order) {
    bqueue q;
    EXPECT_EQ(bq_init(&q), 0);
    for (int i = 1; i <= 8; i++) EXPECT_EQ(bq_push(&q, i * 10), 0);
    EXPECT_EQ(bq_size(&q), 8);
    int v = 0;
    for (int i = 1; i <= 5; i++) { EXPECT_EQ(bq_pop(&q, &v), 0); EXPECT_EQ(v, i * 10); }
    for (int i = 9; i <= 13; i++) EXPECT_EQ(bq_push(&q, i * 10), 0);   /* wraps around the ring */
    for (int i = 6; i <= 13; i++) { EXPECT_EQ(bq_pop(&q, &v), 0); EXPECT_EQ(v, i * 10); }
    EXPECT_EQ(bq_size(&q), 0);
    bq_destroy(&q);
}

struct waiter { bqueue *q; int value; int rc; volatile bool done; };

/* pthread_join with a deadline, so a thread that never wakes fails the test instead of hanging it. */
static bool join_within(pthread_t t, long ms) {
    struct timespec deadline;
    clock_gettime(CLOCK_REALTIME, &deadline);
    deadline.tv_sec += ms / 1000;
    deadline.tv_nsec += (ms % 1000) * 1000000L;
    if (deadline.tv_nsec >= 1000000000L) { deadline.tv_sec++; deadline.tv_nsec -= 1000000000L; }
    return pthread_timedjoin_np(t, NULL, &deadline) == 0;
}

static void *pop_one(void *arg) {
    struct waiter *w = arg;
    w->rc = bq_pop(w->q, &w->value);
    __atomic_store_n(&w->done, true, __ATOMIC_SEQ_CST);
    return NULL;
}

static void *push_one(void *arg) {
    struct waiter *w = arg;
    w->rc = bq_push(w->q, w->value);
    __atomic_store_n(&w->done, true, __ATOMIC_SEQ_CST);
    return NULL;
}

TEST(a_spurious_wakeup_does_not_pop_from_an_empty_queue) {
    bqueue q;
    EXPECT_EQ(bq_init(&q), 0);
    struct waiter w = { .q = &q };
    pthread_t t;
    pthread_create(&t, NULL, pop_one, &w);
    sleep_ms(30);                                   /* the consumer is now waiting */
    for (int i = 0; i < 5; i++) {                   /* wake it up with nothing in the queue */
        pthread_cond_broadcast(&q.not_empty);
        sleep_ms(5);
    }
    bool returned_early = __atomic_load_n(&w.done, __ATOMIC_SEQ_CST);
    bq_push(&q, 42);
    pthread_join(t, NULL);
    EXPECT_FALSE(returned_early);                   /* it must go back to sleep: `while`, not `if` */
    EXPECT_EQ(w.rc, 0);
    EXPECT_EQ(w.value, 42);
    bq_destroy(&q);
}

TEST(a_full_queue_blocks_the_producer_even_on_spurious_wakeups) {
    bqueue q;
    EXPECT_EQ(bq_init(&q), 0);
    for (int i = 0; i < 8; i++) bq_push(&q, i);
    struct waiter w = { .q = &q, .value = 8 };
    pthread_t t;
    pthread_create(&t, NULL, push_one, &w);
    sleep_ms(30);
    for (int i = 0; i < 5; i++) {
        pthread_cond_broadcast(&q.not_full);
        sleep_ms(5);
    }
    bool returned_early = __atomic_load_n(&w.done, __ATOMIC_SEQ_CST);
    size_t size_while_blocked = bq_size(&q);
    int v = -1;
    bq_pop(&q, &v);                                 /* make room: the producer can finish now */
    pthread_join(t, NULL);
    EXPECT_FALSE(returned_early);
    EXPECT_EQ(size_while_blocked, 8);
    EXPECT_EQ(v, 0);                                /* the oldest item wasn't overwritten */
    for (int i = 1; i <= 8; i++) { bq_pop(&q, &v); EXPECT_EQ(v, i); }
    bq_destroy(&q);
}

TEST(close_wakes_every_blocked_consumer_and_drains_first) {
    bqueue q;
    EXPECT_EQ(bq_init(&q), 0);
    struct waiter w[3];
    pthread_t t[3];
    for (int i = 0; i < 3; i++) {
        w[i] = (struct waiter){ .q = &q, .value = -1 };
        pthread_create(&t[i], NULL, pop_one, &w[i]);
    }
    sleep_ms(30);
    bq_close(&q);
    for (int i = 0; i < 3; i++) {
        EXPECT_TRUE(join_within(t[i], 500));        /* broadcast, not signal: wake them all */
        EXPECT_EQ(w[i].rc, -EPIPE);
    }
    EXPECT_EQ(bq_push(&q, 1), -EPIPE);              /* no pushes after close */
    bq_destroy(&q);

    EXPECT_EQ(bq_init(&q), 0);                      /* items pushed before close still come out */
    bq_push(&q, 7);
    bq_push(&q, 8);
    bq_close(&q);
    int v = 0;
    EXPECT_EQ(bq_pop(&q, &v), 0);
    EXPECT_EQ(v, 7);
    EXPECT_EQ(bq_pop(&q, &v), 0);
    EXPECT_EQ(v, 8);
    EXPECT_EQ(bq_pop(&q, &v), -EPIPE);
    bq_destroy(&q);
}

#define PRODUCERS 4
#define CONSUMERS 3
#define PER_PRODUCER 4000

static unsigned char seen[PRODUCERS * PER_PRODUCER];
static int order_errors;

struct producer { bqueue *q; int id; };

static void *produce(void *arg) {
    struct producer *p = arg;
    for (int i = 0; i < PER_PRODUCER; i++) bq_push(p->q, p->id * PER_PRODUCER + i);
    return NULL;
}

static void *consume(void *arg) {
    bqueue *q = arg;
    int last[PRODUCERS];
    for (int i = 0; i < PRODUCERS; i++) last[i] = -1;
    int v;
    while (bq_pop(q, &v) == 0) {
        if (v < 0 || v >= PRODUCERS * PER_PRODUCER) { __atomic_fetch_add(&order_errors, 1, __ATOMIC_SEQ_CST); continue; }
        __atomic_fetch_add(&seen[v], 1, __ATOMIC_SEQ_CST);
        int p = v / PER_PRODUCER;
        if (v <= last[p]) __atomic_fetch_add(&order_errors, 1, __ATOMIC_SEQ_CST);   /* FIFO per producer */
        last[p] = v;
    }
    return NULL;
}

TEST(many_producers_and_consumers_never_lose_or_duplicate_an_item) {
    bqueue q;
    EXPECT_EQ(bq_init(&q), 0);
    pthread_t pt[PRODUCERS], ct[CONSUMERS];
    struct producer p[PRODUCERS];
    for (int i = 0; i < CONSUMERS; i++) pthread_create(&ct[i], NULL, consume, &q);
    for (int i = 0; i < PRODUCERS; i++) {
        p[i] = (struct producer){ &q, i };
        pthread_create(&pt[i], NULL, produce, &p[i]);
    }
    for (int i = 0; i < PRODUCERS; i++) pthread_join(pt[i], NULL);
    bq_close(&q);                                   /* consumers drain the rest, then see -EPIPE */
    for (int i = 0; i < CONSUMERS; i++) pthread_join(ct[i], NULL);
    int missing = 0, duplicated = 0;
    for (int i = 0; i < PRODUCERS * PER_PRODUCER; i++) {
        if (seen[i] == 0) missing++;
        if (seen[i] > 1) duplicated++;
    }
    EXPECT_EQ(missing, 0);
    EXPECT_EQ(duplicated, 0);
    EXPECT_EQ(order_errors, 0);
    bq_destroy(&q);
}
