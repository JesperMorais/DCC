A camera thread produces frame indices and a few worker threads consume them. Between them sits a **bounded queue** with 8 slots. It's the Fundamentals queue, rebuilt from POSIX parts: a mutex and two condition variables. The control thread will run at `SCHED_FIFO` on the target, so the mutex must use **priority inheritance**, or you're back on Mars Pathfinder.

The struct is in the starter. Keep its field names, because the tests reach into the condition variables to fake spurious wakeups.

```c
int    bq_init(bqueue *q);              /* 0, or -error */
void   bq_destroy(bqueue *q);
int    bq_push(bqueue *q, int value);   /* blocks while full */
int    bq_pop(bqueue *q, int *out);     /* blocks while empty */
void   bq_close(bqueue *q);
size_t bq_size(bqueue *q);
```

- **`bq_init`** initialises `lock` as a `PTHREAD_PRIO_INHERIT` mutex, plus both condition variables. The tests check the protocol inside the mutex itself.
- **`bq_push`** blocks while the queue is full, then appends `value`. It returns 0, or `-EPIPE` once the queue is closed.
- **`bq_pop`** blocks while the queue is empty, then removes the **oldest** item into `*out`. It returns 0, or `-EPIPE` when the queue is closed **and** empty. Items pushed before the close still come out.
- **`bq_close`** wakes **every** blocked thread. Pushes fail from then on, and pops drain what's left.
- **`bq_size`** returns the number of items, read under the lock.

**Spurious wakeups are real.** `pthread_cond_wait` may return even though nobody pushed anything, and the tests do exactly that with `pthread_cond_broadcast`. A woken thread must check the condition again, and the queue must never pop from an empty buffer or overwrite a full one.

The stress test runs 4 producers and 3 consumers with 16,000 items. Every item must arrive **exactly once**, and each producer's items must arrive in order.

You don't need root. `PTHREAD_PRIO_INHERIT` works for normal users. Only *setting* `SCHED_FIFO` needs privileges, and nothing here does that.
