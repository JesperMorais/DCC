/* Zephyr kernel API facade over the daily.ts simulator. */
#define _GNU_SOURCE
#include <string.h>

#include "zephyr/kernel.h"

/* Zephyr prio (lower = more urgent, <0 = cooperative) → simulator prio (higher = more urgent). */
static int to_sim(int zprio) { return 100 - zprio; }
static int from_sim(int sprio) { return 100 - sprio; }
/* A relative timeout waits one extra tick, as in real Zephyr (kernel/timeout.c adds 1 so a wait is never
 * shorter than asked, even when it starts just before a tick). k_timer compensates, so timers stay on their grid. */
static rtos_tick_t ticks_of(k_timeout_t t) {
    if (t.ticks == -1) return RTOS_WAIT_FOREVER;
    if (t.ticks < -1) { /* absolute: wait until that tick, at least one tick */
        int64_t until = -2 - t.ticks, now = (int64_t)rtos_now();
        return until > now ? (rtos_tick_t)(until - now) : 1;
    }
    return t.ticks == 0 ? 0 : (rtos_tick_t)t.ticks + 1;
}

/* Zephyr defaults: CONFIG_TIMESLICING=y, CONFIG_TIMESLICE_SIZE=20 (ms) for preemptible threads of equal priority. */
static void z_mode(void) {
    static bool done;
    if (!done) {
        done = true;
        sim_set_time_slice(20);
    }
}

static void check_prio(int prio) {
    if (prio < -CONFIG_NUM_COOP_PRIORITIES || prio > K_LOWEST_APPLICATION_THREAD_PRIO)
        sim_fail("thread priority %d is out of range (%d..%d)", prio, -CONFIG_NUM_COOP_PRIORITIES, K_LOWEST_APPLICATION_THREAD_PRIO);
}

static void thread_tramp(void *arg) {
    struct k_thread *t = arg;
    t->entry(t->p1, t->p2, t->p3);
}

k_tid_t k_thread_create(struct k_thread *thread, void *stack, size_t stack_size, k_thread_entry_t entry, void *p1, void *p2, void *p3,
                        int prio, uint32_t options, k_timeout_t delay) {
    (void)stack;
    (void)options;
    if (!thread) sim_fail("k_thread_create(NULL, ...)");
    z_mode();
    if (stack_size < 256) sim_fail("a %zu-byte thread stack is too small", stack_size);
    check_prio(prio);
    thread->entry = entry;
    thread->p1 = p1;
    thread->p2 = p2;
    thread->p3 = p3;
    rtos_tick_t d = ticks_of(delay); /* a start delay is a relative timeout too: +1 tick */
    if (d == RTOS_WAIT_FOREVER) sim_fail("K_FOREVER start delays aren't supported in the simulator");
    thread->task = sim_task_create_ex("thread", thread_tramp, thread, to_sim(prio), prio < 0, d);
    return thread;
}

void k_thread_name_set(k_tid_t thread, const char *name) {
    /* The simulator names tasks at creation; renaming isn't tracked, but keep the API. */
    (void)thread;
    (void)name;
}

static struct { struct k_thread *t; const char *name; k_thread_entry_t entry; void *p1, *p2, *p3; int prio, delay; } statics[32];
static int nstatics;
static void start_statics(void) {
    for (int i = 0; i < nstatics; i++) {
        struct k_thread *t = statics[i].t;
        check_prio(statics[i].prio);
        t->entry = statics[i].entry;
        t->p1 = statics[i].p1;
        t->p2 = statics[i].p2;
        t->p3 = statics[i].p3;
        rtos_tick_t delay = statics[i].delay > 0 ? (rtos_tick_t)statics[i].delay + 1 : 0; /* as on real Zephyr: 50 ms starts at 51 */
        t->task = sim_task_create_ex(statics[i].name, thread_tramp, t, to_sim(statics[i].prio), statics[i].prio < 0, delay);
    }
}
void dts_z_register_static(struct k_thread *t, const char *name, k_thread_entry_t entry, void *p1, void *p2, void *p3, int prio, int delay_ms) {
    z_mode();
    if (nstatics == 0) sim_on_start(start_statics);
    if (nstatics < 32) statics[nstatics++] = (typeof(statics[0])){t, name, entry, p1, p2, p3, prio, delay_ms};
}

/* Map the current simulator task back to its k_thread (or NULL for the system workqueue). */
static struct k_thread *self_thread(void) {
    rtos_task_t *me = rtos_self();
    for (int i = 0; i < nstatics; i++)
        if (statics[i].t->task == me) return statics[i].t;
    return NULL;
}
k_tid_t k_current_get(void) { return self_thread(); }
int k_thread_priority_get(k_tid_t t) { return from_sim(rtos_task_priority(t ? t->task : rtos_self())); }
void k_thread_priority_set(k_tid_t t, int prio) {
    check_prio(prio);
    rtos_task_set_priority(t ? t->task : rtos_self(), to_sim(prio));
}

int32_t k_sleep(k_timeout_t timeout) {
    if (sim_in_isr()) sim_fail("k_sleep() in an ISR — ISRs must never sleep");
    rtos_tick_t t = ticks_of(timeout);
    if (t == RTOS_WAIT_FOREVER) sim_fail("k_sleep(K_FOREVER) suspends the thread forever — that's probably not what you want here");
    rtos_delay(t);
    return 0;
}
int32_t k_msleep(int32_t ms) { return k_sleep(K_MSEC(ms)); }
void k_yield(void) { rtos_yield(); }
int64_t k_uptime_get(void) { return (int64_t)rtos_now(); }
uint32_t k_uptime_get_32(void) { return rtos_now(); }
void k_busy_wait(uint32_t usec) { rtos_busy((usec + 999) / 1000); }

/* ---- semaphores ---- */
static void sem_lazy(struct k_sem *s) {
    if (!s->s) s->s = rtos_sem_create(s->init, s->limit ? s->limit : 1);
}
int k_sem_init(struct k_sem *s, unsigned initial, unsigned limit) {
    if (!s || limit == 0 || initial > limit) return -EINVAL;
    s->init = initial;
    s->limit = limit;
    s->s = rtos_sem_create(initial, limit);
    return 0;
}
int k_sem_take(struct k_sem *s, k_timeout_t timeout) {
    sem_lazy(s);
    if (sim_in_isr() && timeout.ticks != 0) sim_fail("k_sem_take() with a timeout in an ISR would block — use K_NO_WAIT");
    if (rtos_sem_take(s->s, ticks_of(timeout))) return 0;
    return timeout.ticks == 0 ? -EBUSY : -EAGAIN;
}
void k_sem_give(struct k_sem *s) {
    sem_lazy(s);
    rtos_sem_give(s->s);
}
unsigned k_sem_count_get(struct k_sem *s) {
    sem_lazy(s);
    return rtos_sem_count(s->s);
}
void k_sem_reset(struct k_sem *s) {
    sem_lazy(s);
    while (rtos_sem_take(s->s, RTOS_NO_WAIT)) {
    }
}

/* ---- mutexes ---- */
static void mutex_lazy(struct k_mutex *m) {
    if (!m->m) m->m = rtos_mutex_create(true);
}
int k_mutex_init(struct k_mutex *m) {
    m->m = rtos_mutex_create(true);
    m->lock_count = 0;
    return 0;
}
int k_mutex_lock(struct k_mutex *m, k_timeout_t timeout) {
    mutex_lazy(m);
    if (sim_in_isr()) sim_fail("k_mutex_lock() in an ISR — mutexes are for threads only");
    if (rtos_mutex_owner(m->m) == rtos_self()) {
        m->lock_count++; /* Zephyr mutexes are recursive */
        return 0;
    }
    if (!rtos_mutex_lock(m->m, ticks_of(timeout))) return timeout.ticks == 0 ? -EBUSY : -EAGAIN;
    m->lock_count = 1;
    return 0;
}
int k_mutex_unlock(struct k_mutex *m) {
    mutex_lazy(m);
    if (rtos_mutex_owner(m->m) == NULL) return -EINVAL;
    if (rtos_mutex_owner(m->m) != rtos_self()) return -EPERM;
    if (--m->lock_count == 0) rtos_mutex_unlock(m->m);
    return 0;
}

/* ---- message queues ---- */
static void msgq_lazy(struct k_msgq *q) {
    if (!q->q) {
        q->q = rtos_queue_create(q->max_msgs, q->msg_size);
        sim_queue_handoff(q->q, true); /* k_msgq_put copies straight into a waiting k_msgq_get */
    }
}
void k_msgq_init(struct k_msgq *q, char *buffer, size_t msg_size, uint32_t max_msgs) {
    (void)buffer;
    q->msg_size = msg_size;
    q->max_msgs = max_msgs;
    q->q = rtos_queue_create(max_msgs, msg_size);
    sim_queue_handoff(q->q, true);
}
int k_msgq_put(struct k_msgq *q, const void *data, k_timeout_t timeout) {
    msgq_lazy(q);
    if (sim_in_isr() && timeout.ticks != 0) sim_fail("k_msgq_put() with a timeout in an ISR would block — use K_NO_WAIT");
    if (rtos_queue_send(q->q, data, ticks_of(timeout))) return 0;
    return timeout.ticks == 0 ? -ENOMSG : -EAGAIN;
}
int k_msgq_get(struct k_msgq *q, void *data, k_timeout_t timeout) {
    msgq_lazy(q);
    if (sim_in_isr() && timeout.ticks != 0) sim_fail("k_msgq_get() with a timeout in an ISR would block — use K_NO_WAIT");
    if (rtos_queue_receive(q->q, data, ticks_of(timeout))) return 0;
    return timeout.ticks == 0 ? -ENOMSG : -EAGAIN;
}
uint32_t k_msgq_num_used_get(struct k_msgq *q) {
    msgq_lazy(q);
    return (uint32_t)rtos_queue_count(q->q);
}
uint32_t k_msgq_num_free_get(struct k_msgq *q) {
    msgq_lazy(q);
    return q->max_msgs - (uint32_t)rtos_queue_count(q->q);
}

/* ---- system workqueue ---- */
static rtos_queue_t *workq;
static void workq_thread(void *arg) {
    (void)arg;
    for (;;) {
        struct k_work *w;
        rtos_queue_receive(workq, &w, RTOS_WAIT_FOREVER);
        w->pending = false;
        w->running = true;
        w->handler(w);
        w->running = false;
    }
}
static void ensure_workq(void) {
    if (workq) return;
    workq = rtos_queue_create(32, sizeof(struct k_work *));
    sim_task_create_ex("sysworkq", workq_thread, NULL, to_sim(CONFIG_SYSTEM_WORKQUEUE_PRIORITY), true, 0);
}
/* The system workqueue thread exists from boot whenever the app uses work items. */
void dts_z_work_used(void) {
    static bool registered;
    if (!registered) {
        registered = true;
        sim_on_start(ensure_workq);
    }
}
void k_work_init(struct k_work *w, k_work_handler_t handler) {
    w->handler = handler;
    w->pending = false;
    w->running = false;
    if (sim_in_isr()) dts_z_work_used();
    else ensure_workq();
}
int k_work_submit(struct k_work *w) {
    if (!w || !w->handler) sim_fail("k_work_submit() on an uninitialised work item — call k_work_init() first");
    if (!workq) sim_fail("k_work_submit() before the workqueue exists — initialise the work item with k_work_init() or K_WORK_DEFINE");
    if (w->pending) return 0;
    w->pending = true;
    if (!rtos_queue_send(workq, &w, RTOS_NO_WAIT)) sim_fail("the system workqueue is full");
    return w->running ? 2 : 1;
}
bool k_work_is_pending(const struct k_work *w) { return w && w->pending; }

/* ---- timers ---- */
static void timer_cb(void *arg) {
    struct k_timer *t = arg;
    t->status++;
    if (t->expiry) t->expiry(t);
}
void k_timer_init(struct k_timer *t, k_timer_expiry_t expiry, k_timer_stop_t stop) {
    t->expiry = expiry;
    t->stop = stop;
    t->status = 0;
    t->t = NULL;
}
void k_timer_start(struct k_timer *t, k_timeout_t duration, k_timeout_t period) {
    bool periodic = period.ticks > 0;
    rtos_tick_t first = duration.ticks <= 0 ? 1 : (rtos_tick_t)duration.ticks;
    if (t->t) rtos_timer_stop(t->t); /* restarting a running timer: the old schedule is cancelled, no stop fn */
    t->t = rtos_timer_create("k_timer", periodic ? (rtos_tick_t)period.ticks : first, periodic, timer_cb, t);
    t->status = 0;
    rtos_timer_start(t->t, first);
}
void k_timer_stop(struct k_timer *t) {
    bool was_running = rtos_timer_is_active(t->t);
    if (t->t) rtos_timer_stop(t->t);
    if (was_running && t->stop) t->stop(t); /* Zephyr only calls the stop fn if the timer was running */
}
uint32_t k_timer_status_get(struct k_timer *t) {
    uint32_t s = t->status;
    t->status = 0;
    return s;
}
