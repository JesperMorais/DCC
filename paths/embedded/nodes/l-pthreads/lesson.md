In 1997 Mars Pathfinder kept rebooting itself on the surface of Mars. A low-priority meteorology task held a mutex, a medium-priority task kept it off the CPU, and the high-priority bus task waited on that mutex until the watchdog gave up. JPL fixed it from 190 million kilometres away by flipping one flag: **priority inheritance** on that mutex.

On Linux you'll write the same code with pthreads, and you'll find the same trap. The default mutex has the flag **off**.

### From RTOS tasks to pthreads

The Fundamentals concepts map one-to-one onto pthreads:

| Fundamentals | POSIX threads |
|---|---|
| `rtos_task_create(name, fn, arg, prio)` | `pthread_create(&t, &attr, fn, arg)` |
| a task that finishes | `pthread_join(t, &ret)` collects it |
| `rtos_mutex_create(true)` | a mutex with `PTHREAD_PRIO_INHERIT` |
| a queue with blocking send/receive | a mutex + **two condition variables** |

```c
void *worker(void *arg) { /* ... */ return NULL; }

pthread_t t;
pthread_create(&t, NULL, worker, &ctx);   // starts running right away
pthread_join(t, NULL);                    // wait for it to finish
```

Every thread you create must be joined (or detached), or it leaks like a `malloc` you never freed.

### Condition variables: "sleep until the state changes"

A mutex protects data. A **condition variable** lets a thread sleep until that data reaches a state it cares about, such as "the queue isn't empty". It only works together with the mutex:

```c
pthread_mutex_lock(&q->lock);
while (q->count == 0)                       // the condition you're waiting for
    pthread_cond_wait(&q->not_empty, &q->lock);
/* here: count > 0, and you hold the lock */
```

`pthread_cond_wait` does three things **atomically**: it unlocks the mutex, sleeps, and re-locks the mutex before it returns. Because the unlock and the sleep happen together, a producer can't slip its signal into the gap between them and get lost.

The producer changes the state under the same lock and then signals:

```c
pthread_mutex_lock(&q->lock);
/* ... add an item ... */
pthread_cond_signal(&q->not_empty);
pthread_mutex_unlock(&q->lock);
```

A bounded queue needs **two** condition variables: `not_empty` for consumers waiting on an empty queue, and `not_full` for producers waiting on a full one. That gives you the Fundamentals `rtos_queue_send`/`receive` with `RTOS_WAIT_FOREVER`, built from parts.

### Why it's `while`, never `if`

There are two separate reasons.

1. **Spurious wakeups.** POSIX explicitly allows `pthread_cond_wait` to return when nobody signalled. Futex-based implementations really do this, for example around signals.
2. **Stolen wakeups.** This one bites even without spurious wakeups. Picture the timeline:

```
consumer A: queue empty → cond_wait (sleeping)
producer:   push item, signal → A is now runnable but not yet running
consumer B: arrives, grabs the lock, sees count == 1, pops it
consumer A: finally re-acquires the lock... count == 0
```

With `if`, consumer A pops from an empty ring. It reads garbage, and `count` wraps around to 18 quintillion. With `while`, A simply checks the condition, sees it's false and goes back to sleep. A wakeup only means "something *may* have changed, so go and look."

### signal or broadcast?

`pthread_cond_signal` wakes at least one waiter, and `pthread_cond_broadcast` wakes all of them. Use signal when any one waiter can make progress, such as one item for one consumer. Use broadcast when **everyone** has to re-check: the classic case is a "closed" or shutdown flag, because every blocked consumer must wake up, see `closed` and return. Shutting down with `signal` wakes one thread and leaves the rest asleep forever. Your `pthread_join` then hangs, and so does `systemctl stop`.

### Priority inheritance is opt-in

A glibc `PTHREAD_MUTEX_INITIALIZER` mutex uses `PTHREAD_PRIO_NONE`. The fast path is a single atomic instruction in userspace, and the kernel isn't involved until there's contention. A PI mutex has to tell the kernel *who* owns it (a PI-futex, `FUTEX_LOCK_PI`), so that the kernel can boost the owner. That bookkeeping costs a little, and most desktop programs don't need it, so the default is off. On a real-time system you turn it on:

```c
pthread_mutexattr_t a;
pthread_mutexattr_init(&a);
pthread_mutexattr_setprotocol(&a, PTHREAD_PRIO_INHERIT);
pthread_mutex_init(&m, &a);
pthread_mutexattr_destroy(&a);   // m keeps its own copy
```

This is exactly `rtos_mutex_create(true)` from Fundamentals. It needs no privileges.

### SCHED_FIFO, briefly

Priorities only matter once threads run under a real-time policy. Normal threads use `SCHED_OTHER` (CFS/EEVDF), and there "priority" is just a fairness weight. To get a Fundamentals-style fixed-priority preemptive thread, you'd write:

```c
pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);  // else the attr is ignored!
pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
struct sched_param sp = { .sched_priority = 80 };              // 1..99, higher = more urgent
pthread_attr_setschedparam(&attr, &sp);
```

This needs `CAP_SYS_NICE`, or an `rtprio` limit in `/etc/security/limits.conf`. Without it, `pthread_create` fails with `EPERM`. Forgetting `PTHREAD_EXPLICIT_SCHED` is the classic silent bug: the call succeeds, and the thread quietly inherits `SCHED_OTHER`. The lab doesn't need root. The PI mutex works for everyone, and it's what saves you once the threads *are* SCHED_FIFO on the target.

### Gotchas

- **Waiting on a condition with the wrong mutex,** or changing the state without the lock, brings back lost wakeups.
- **Always pair a state change with the signal of the *other* condition:** a pop frees a slot, so it signals `not_full`.
- **`pthread_*` functions return the error number** (for example `EINVAL`). They don't set `errno`.
- **Don't hold a lock while you do slow work** (I/O, `printf`). With PI on, you'd be lending your high priority to a `write()`.

### In the wild

- **GStreamer, ROS 2 executors and most camera pipelines** are bounded queues between threads, built exactly like this one. Get it wrong and you get frame drops or deadlocks at shutdown.
- **Interview classic:** "Why must `pthread_cond_wait` be in a loop?" Answer with both spurious *and* stolen wakeups, and mention broadcast-on-shutdown.
- **PREEMPT_RT systems** (robot arms, PLCs, LinuxCNC) audit every lock that an RT thread touches, and they insist on `PTHREAD_PRIO_INHERIT`. Pathfinder's bug is just as possible in a Linux userspace daemon as it was in VxWorks.
