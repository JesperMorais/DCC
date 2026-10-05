/* daily.ts RTOS simulator: a deterministic fixed-priority preemptive kernel on ucontext.
 *
 * Model
 *   - Virtual time, 1 tick = 1 ms. Time only advances inside rtos_busy() or while the CPU idles.
 *   - At every tick boundary: timeouts expire, timers fire, scheduled IRQs run (ISR context),
 *     then the scheduler checks for preemption (higher priority ready, or time-slice rotation).
 *   - Blocking calls hand control to the scheduler; give/send/unlock hand the resource
 *     directly to the highest-priority (then longest-waiting) waiter.
 *   - Everything is recorded in a trace that the UI draws as a timeline.
 */
#define _GNU_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

#include "rtos.h"
#include "sim.h"

void dts_fail_msg(const char *msg); /* harness: report a failure for the current test and exit */

#define MAX_TASKS 48
#define MAX_IRQS 64
#define MAX_TIMERS 32
#define MAX_TRACE_TICKS 20000
#define MAX_EVENTS 4000
#define STACK_SIZE (128 * 1024)

enum state { READY, RUNNING, BLOCKED, DONE };
enum wait { W_NONE, W_DELAY, W_SEM, W_MUTEX, W_QRECV, W_QSEND, W_NOTIFY };

struct rtos_task {
    char name[32];
    rtos_task_fn fn;
    void *arg;
    int base_prio, prio, max_prio_seen;
    bool coop;
    enum state state;
    enum wait wait;
    void *wait_obj;
    void *wait_buf; /* queue item source/destination while blocked */
    bool got_item;  /* a handoff queue copied an item straight into wait_buf */
    bool timed, timed_out;
    rtos_tick_t wake_at;
    rtos_tick_t eligible_at; /* woken by an ISR without a yield: runs from the next tick */
    unsigned long long seq;  /* FIFO order among equal priorities */
    uint32_t notify;
    rtos_tick_t ran;
    rtos_tick_t slice_used; /* ticks run since it last got the CPU */
    ucontext_t ctx;
    void *stack;
    int index;
};

struct rtos_sem { unsigned count, max; };
struct rtos_mutex { rtos_task_t *owner; bool pi; };
/* handoff: Zephyr copies a message straight into a waiting receiver. FreeRTOS (and the neutral RTOS) store it
 * in the queue and wake the receiver, which takes it when it runs, so a burst never gets an extra slot. */
struct rtos_queue { unsigned char *buf; size_t len, size, head, count; bool handoff; };
struct rtos_timer { char name[32]; rtos_tick_t period, next; bool periodic, active; rtos_timer_fn cb; void *arg; };

struct irq { rtos_tick_t next, period; const char *name; void (*isr)(void); bool pending; };

static struct {
    bool init, started, in_isr, deadlock_reported, time_slicing, isr_auto_yield, isr_yield_requested;
    rtos_tick_t slice; /* round-robin quantum in ticks */
    rtos_task_t *tasks[MAX_TASKS];
    int ntasks;
    rtos_task_t *current;
    ucontext_t sched_ctx;
    rtos_tick_t now, end;
    unsigned long long seq;
    int critical;
    int switches;
    int last_run; /* task index that ran last, -1 idle */
    int isr_woke_prio; /* highest priority an ISR made ready during this ISR, -1 none */
    struct irq irqs[MAX_IRQS];
    int nirqs;
    rtos_timer_t *timers[MAX_TIMERS];
    int ntimers;
    void *allocs[512];
    int nallocs;
    void (*on_start[8])(void);
    int non_start;
    /* trace */
    short trace[MAX_TRACE_TICKS];
    rtos_tick_t traced;
    struct { rtos_tick_t tick; char type[12]; char a[32]; char b[48]; } events[MAX_EVENTS];
    int nevents;
} S;

/* ------------------------------------------------------------------ helpers */
void *sim_calloc(size_t n);
static void *sim_alloc(size_t n) {
    void *p = calloc(1, n);
    if (!p || S.nallocs >= (int)(sizeof S.allocs / sizeof S.allocs[0])) sim_fail("simulator out of memory (too many RTOS objects)");
    S.allocs[S.nallocs++] = p;
    return p;
}

void *sim_calloc(size_t n) { return sim_alloc(n); }

static void ev(const char *type, const char *a, const char *b) {
    if (S.nevents >= MAX_EVENTS) return;
    S.events[S.nevents].tick = S.now;
    snprintf(S.events[S.nevents].type, sizeof S.events[0].type, "%s", type);
    snprintf(S.events[S.nevents].a, sizeof S.events[0].a, "%s", a ? a : "");
    snprintf(S.events[S.nevents].b, sizeof S.events[0].b, "%s", b ? b : "");
    S.nevents++;
}

static void ensure_init(void) {
    if (S.init) return;
    S.init = true;
    S.time_slicing = true;
    S.slice = 1;
    S.isr_auto_yield = true;
    S.last_run = -2;
}

void sim_fail(const char *fmt, ...) {
    char msg[600], full[800];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);
    const char *who = S.in_isr ? "an ISR" : S.current ? S.current->name : "the test";
    snprintf(full, sizeof full, "RTOS: %s (in %s, at tick %u)", msg, who, (unsigned)S.now);
    ev("fail", who, msg);
    dts_fail_msg(full);
    _exit(0); /* not reached */
}

static void require_task(const char *api) {
    if (S.in_isr) sim_fail("%s() blocks, and blocking calls are not allowed in an ISR — signal a task instead", api);
    if (!S.current) sim_fail("%s() must be called from a task, not from the test before sim_run()", api);
}

/* ------------------------------------------------------------------ scheduling core */
static void set_prio(rtos_task_t *t, int p);
static int inherited_prio(const rtos_task_t *t);
static bool runnable(const rtos_task_t *t) { return t->state == READY && t->eligible_at <= S.now; }

static rtos_task_t *pick(void) {
    rtos_task_t *best = NULL;
    for (int i = 0; i < S.ntasks; i++) {
        rtos_task_t *t = S.tasks[i];
        if (!runnable(t)) continue;
        if (!best || t->prio > best->prio || (t->prio == best->prio && t->seq < best->seq)) best = t;
    }
    return best;
}

static void make_ready(rtos_task_t *t, bool from_isr) {
    t->state = READY;
    t->wait = W_NONE;
    t->wait_obj = NULL;
    t->timed = false;
    t->seq = ++S.seq;
    t->eligible_at = (from_isr && !S.isr_auto_yield) ? S.now + 1 : S.now;
    if (from_isr && t->prio > S.isr_woke_prio) S.isr_woke_prio = t->prio;
    ev("ready", t->name, "");
}

static void switch_to_scheduler(void) {
    rtos_task_t *me = S.current;
    swapcontext(&me->ctx, &S.sched_ctx);
}

/* Should the running task give up the CPU at this point? */
static bool should_preempt(const rtos_task_t *cur) {
    if (S.critical) return false;
    for (int i = 0; i < S.ntasks; i++) {
        const rtos_task_t *t = S.tasks[i];
        if (t == cur || !runnable(t)) continue;
        if (!cur->coop && t->prio > cur->prio) return true;
    }
    return false;
}

static bool should_rotate(const rtos_task_t *cur) {
    if (S.critical || !S.time_slicing || cur->coop) return false;
    for (int i = 0; i < S.ntasks; i++) {
        const rtos_task_t *t = S.tasks[i];
        if (t != cur && runnable(t) && t->prio == cur->prio) return true;
    }
    return false;
}

static void preempt_if_needed(void) {
    rtos_task_t *cur = S.current;
    if (cur && !S.in_isr && should_preempt(cur)) {
        cur->state = READY; /* keeps its seq: it resumes first among equals */
        ev("preempt", cur->name, "");
        switch_to_scheduler();
    }
}

static void run_isr(struct irq *q) {
    if (S.critical) {
        q->pending = true;
        return;
    }
    bool was = S.in_isr;
    S.in_isr = true;
    S.isr_yield_requested = false;
    S.isr_woke_prio = -1;
    ev("irq", q->name, "");
    q->isr();
    if (S.isr_yield_requested) {
        for (int i = 0; i < S.ntasks; i++)
            if (S.tasks[i]->state == READY && S.tasks[i]->eligible_at > S.now) S.tasks[i]->eligible_at = S.now;
    }
    S.in_isr = was;
}

static void fire_timer(rtos_timer_t *t) {
    bool was = S.in_isr;
    S.in_isr = true;
    S.isr_yield_requested = false;
    S.isr_woke_prio = -1;
    ev("timer", t->name, "");
    t->cb(t->arg);
    if (S.isr_yield_requested)
        for (int i = 0; i < S.ntasks; i++)
            if (S.tasks[i]->state == READY && S.tasks[i]->eligible_at > S.now) S.tasks[i]->eligible_at = S.now;
    S.in_isr = was;
}

/* Everything that happens exactly at tick boundary S.now. */
static void tick_events(void) {
    for (int i = 0; i < S.ntasks; i++) {
        rtos_task_t *t = S.tasks[i];
        if (t->state == BLOCKED && t->timed && t->wake_at <= S.now) {
            if (t->wait == W_DELAY) {
                make_ready(t, false);
            } else {
                t->timed_out = true;
                ev("timeout", t->name, "");
                rtos_mutex_t *m = t->wait == W_MUTEX ? t->wait_obj : NULL;
                make_ready(t, false);
                if (m && m->owner) set_prio(m->owner, inherited_prio(m->owner)); /* stop lending priority */
            }
        }
    }
    for (int i = 0; i < S.ntimers; i++) {
        rtos_timer_t *t = S.timers[i];
        if (t->active && t->next == S.now) {
            if (t->periodic) t->next += t->period;
            else t->active = false;
            fire_timer(t);
        }
    }
    for (int i = 0; i < S.nirqs; i++) {
        struct irq *q = &S.irqs[i];
        if (q->next == S.now && q->isr) {
            if (q->period) q->next += q->period;
            else q->next = RTOS_WAIT_FOREVER;
            run_isr(q);
        }
    }
}

static void record_tick(int who) {
    if (S.now < MAX_TRACE_TICKS) S.trace[S.now] = (short)who;
    if (S.now + 1 > S.traced) S.traced = S.now + 1;
}

static bool future_events_exist(void) {
    for (int i = 0; i < S.ntasks; i++)
        if (S.tasks[i]->state == BLOCKED && S.tasks[i]->timed) return true;
    for (int i = 0; i < S.ntimers; i++)
        if (S.timers[i]->active) return true;
    for (int i = 0; i < S.nirqs; i++)
        if (S.irqs[i].next != RTOS_WAIT_FOREVER && S.irqs[i].isr) return true;
    for (int i = 0; i < S.ntasks; i++)
        if (S.tasks[i]->state == READY) return true;
    return false;
}

static void trampoline(void) {
    rtos_task_t *me = S.current;
    me->fn(me->arg);
    rtos_task_exit();
}

/* ------------------------------------------------------------------ public: control */
void sim_set_time_slicing(bool on) { ensure_init(); S.time_slicing = on; }
void sim_set_time_slice(rtos_tick_t ticks) { ensure_init(); S.slice = ticks ? ticks : 1; }
void sim_set_isr_auto_yield(bool on) { ensure_init(); S.isr_auto_yield = on; }
void sim_isr_yield(void) { S.isr_yield_requested = true; }
/* Did this ISR make ready a task that outranks the one it interrupted (the task that ran last, or idle)?
 * That's when FreeRTOS's FromISR calls set *pxHigherPriorityTaskWoken. */
bool sim_isr_woke_higher(void) {
    const rtos_task_t *was = S.last_run >= 0 ? S.tasks[S.last_run] : NULL;
    int interrupted = was && was->state != BLOCKED && was->state != DONE ? was->prio : -1;
    return S.isr_woke_prio > interrupted;
}
bool sim_in_isr(void) { return S.in_isr; }
void sim_on_start(void (*fn)(void)) {
    ensure_init();
    if (S.non_start < 8) S.on_start[S.non_start++] = fn;
}

void sim_run(rtos_tick_t ticks) {
    ensure_init();
    if (S.current) sim_fail("sim_run() was called from inside a task");
    if (!S.started) {
        S.started = true;
        for (int i = 0; i < S.non_start; i++) S.on_start[i]();
        tick_events(); /* events scheduled at tick 0 */
    }
    S.end = S.now + ticks;
    while (S.now < S.end) {
        rtos_task_t *t = pick();
        if (!t) {
            if (!S.deadlock_reported && !future_events_exist()) {
                bool any_blocked = false;
                for (int i = 0; i < S.ntasks; i++) any_blocked |= S.tasks[i]->state == BLOCKED;
                if (any_blocked) {
                    S.deadlock_reported = true;
                    ev("deadlock", "", "every task is blocked forever");
                }
            }
            record_tick(-1);
            if (S.last_run != -1) S.last_run = -1;
            S.now++;
            tick_events();
            continue;
        }
        if (S.last_run != t->index) {
            t->slice_used = 0; /* a fresh time slice each time it's switched in */
            if (S.last_run >= -1) S.switches++;
            S.last_run = t->index;
            ev("run", t->name, "");
        }
        S.current = t;
        t->state = RUNNING;
        swapcontext(&S.sched_ctx, &t->ctx);
        S.current = NULL;
    }
}

void sim_irq_at(rtos_tick_t tick, const char *name, void (*isr)(void)) {
    ensure_init();
    if (S.nirqs >= MAX_IRQS) sim_fail("too many simulated interrupts");
    S.irqs[S.nirqs++] = (struct irq){tick, 0, name, isr, false};
}

void sim_irq_every(rtos_tick_t first, rtos_tick_t period, const char *name, void (*isr)(void)) {
    ensure_init();
    if (S.nirqs >= MAX_IRQS) sim_fail("too many simulated interrupts");
    if (period == 0) sim_fail("sim_irq_every needs a period > 0");
    S.irqs[S.nirqs++] = (struct irq){first, period, name, isr, false};
}

static rtos_task_t *find(const char *name) {
    for (int i = 0; i < S.ntasks; i++)
        if (strcmp(S.tasks[i]->name, name) == 0) return S.tasks[i];
    return NULL;
}

rtos_tick_t sim_ran_ticks(const char *name) {
    rtos_task_t *t = find(name);
    return t ? t->ran : 0;
}
const char *sim_running_at(rtos_tick_t tick) {
    if (tick >= S.traced || tick >= MAX_TRACE_TICKS) return "idle";
    int who = S.trace[tick];
    return who < 0 ? "idle" : S.tasks[who]->name;
}
rtos_tick_t sim_first_run_at_or_after(const char *name, rtos_tick_t tick) {
    rtos_task_t *t = find(name);
    if (!t) return RTOS_WAIT_FOREVER;
    for (rtos_tick_t i = tick; i < S.traced && i < MAX_TRACE_TICKS; i++)
        if (S.trace[i] == t->index) return i;
    return RTOS_WAIT_FOREVER;
}
int sim_context_switches(void) { return S.switches; }
bool sim_deadlocked(void) { return S.deadlock_reported; }
bool sim_task_finished(const char *name) {
    rtos_task_t *t = find(name);
    return t && t->state == DONE;
}
int sim_max_priority_seen(const char *name) {
    rtos_task_t *t = find(name);
    return t ? t->max_prio_seen : -1;
}
void sim_mark(const char *label) { ev("mark", S.current ? S.current->name : "", label); }

/* ------------------------------------------------------------------ public: tasks */
rtos_task_t *sim_task_create_ex(const char *name, rtos_task_fn fn, void *arg, int priority, bool coop, rtos_tick_t start_delay) {
    ensure_init();
    if (S.in_isr) sim_fail("tasks can't be created from an ISR");
    if (S.ntasks >= MAX_TASKS) sim_fail("too many tasks (max %d)", MAX_TASKS);
    if (!fn) sim_fail("task \"%s\" has no function", name ? name : "?");
    rtos_task_t *t = sim_alloc(sizeof *t);
    snprintf(t->name, sizeof t->name, "%s", name && *name ? name : "task");
    if (find(t->name)) {
        char buf[32];
        snprintf(buf, sizeof buf, "%.24s#%d", t->name, S.ntasks);
        snprintf(t->name, sizeof t->name, "%s", buf);
    }
    t->fn = fn;
    t->arg = arg;
    t->base_prio = t->prio = t->max_prio_seen = priority;
    t->coop = coop;
    t->index = S.ntasks;
    t->stack = sim_alloc(STACK_SIZE);
    getcontext(&t->ctx);
    t->ctx.uc_stack.ss_sp = t->stack;
    t->ctx.uc_stack.ss_size = STACK_SIZE;
    t->ctx.uc_link = NULL;
    makecontext(&t->ctx, trampoline, 0);
    S.tasks[S.ntasks++] = t;
    ev("create", t->name, "");
    if (start_delay) {
        t->state = BLOCKED;
        t->wait = W_DELAY;
        t->timed = true;
        t->wake_at = S.now + start_delay;
    } else {
        make_ready(t, false);
        /* Creating a higher-priority task from a running task preempts it immediately. */
        if (S.current) preempt_if_needed();
    }
    return t;
}

rtos_task_t *rtos_task_create(const char *name, rtos_task_fn fn, void *arg, int priority) {
    return sim_task_create_ex(name, fn, arg, priority, false, 0);
}

void rtos_task_exit(void) {
    rtos_task_t *me = S.current;
    if (!me) sim_fail("rtos_task_exit() called outside a task");
    me->state = DONE;
    ev("exit", me->name, "");
    switch_to_scheduler();
    sim_fail("a finished task was resumed"); /* not reached */
}

static void block(enum wait w, void *obj, rtos_tick_t timeout) {
    rtos_task_t *me = S.current;
    me->state = BLOCKED;
    me->wait = w;
    me->wait_obj = obj;
    me->timed = timeout != RTOS_WAIT_FOREVER;
    me->wake_at = S.now + timeout;
    me->timed_out = false;
    me->seq = ++S.seq;
    switch_to_scheduler();
}

void rtos_delay(rtos_tick_t ticks) {
    require_task("rtos_delay");
    if (S.critical) sim_fail("rtos_delay() inside a critical section would freeze the whole system");
    if (ticks == 0) {
        rtos_yield();
        return;
    }
    ev("delay", S.current->name, "");
    block(W_DELAY, NULL, ticks);
}

void rtos_delay_until(rtos_tick_t *last_wake, rtos_tick_t period) {
    require_task("rtos_delay_until");
    if (!last_wake) sim_fail("rtos_delay_until() needs a pointer to the last wake time");
    rtos_tick_t next = *last_wake + period;
    *last_wake = next;
    if (next > S.now) {
        ev("delay", S.current->name, "");
        block(W_DELAY, NULL, next - S.now);
    } else {
        ev("overrun", S.current->name, "missed its period");
    }
}

void rtos_yield(void) {
    require_task("rtos_yield");
    rtos_task_t *me = S.current;
    me->state = READY;
    me->seq = ++S.seq; /* go to the back of the line */
    switch_to_scheduler();
}

void rtos_busy(rtos_tick_t ticks) {
    if (S.in_isr) sim_fail("rtos_busy() in an ISR: keep interrupt handlers short and hand the work to a task");
    if (!S.current) sim_fail("rtos_busy() must run inside a task");
    rtos_task_t *me = S.current;
    for (rtos_tick_t i = 0; i < ticks; i++) {
        while (S.now >= S.end) {
            /* Out of simulated time: pause here, still runnable, so the next sim_run() resumes this task.
             * It keeps its seq, so it resumes first among equals. */
            me->state = READY;
            switch_to_scheduler();
        }
        record_tick(me->index);
        me->ran++;
        me->slice_used++;
        S.now++;
        tick_events();
        if (S.critical) continue;
        if (should_preempt(me)) {
            me->state = READY;
            ev("preempt", me->name, "");
            switch_to_scheduler();
        } else if (i + 1 < ticks && me->slice_used >= S.slice && should_rotate(me)) {
            me->state = READY;
            me->seq = ++S.seq;
            switch_to_scheduler();
        }
    }
}

rtos_tick_t rtos_now(void) { return S.now; }
rtos_task_t *rtos_self(void) { return S.current; }
const char *rtos_task_name(const rtos_task_t *t) { return t ? t->name : "?"; }
int rtos_task_priority(const rtos_task_t *t) { return t ? t->prio : -1; }

static void set_prio(rtos_task_t *t, int p) {
    if (t->prio != p) {
        char b[48];
        snprintf(b, sizeof b, "%d -> %d", t->prio, p);
        ev("prio", t->name, b);
    }
    t->prio = p;
    if (p > t->max_prio_seen) t->max_prio_seen = p;
}

void rtos_task_set_priority(rtos_task_t *t, int p) {
    if (!t) sim_fail("rtos_task_set_priority(NULL, ...)");
    t->base_prio = p;
    set_prio(t, p);
    if (S.current) {
        if (S.current == t && !should_preempt(t)) return;
        preempt_if_needed();
    }
}

/* ------------------------------------------------------------------ critical sections */
void rtos_enter_critical(void) {
    if (S.in_isr) return;
    S.critical++;
}

void rtos_exit_critical(void) {
    if (S.in_isr) return;
    if (S.critical <= 0) sim_fail("rtos_exit_critical() without a matching rtos_enter_critical()");
    if (--S.critical == 0) {
        for (int i = 0; i < S.nirqs; i++)
            if (S.irqs[i].pending) {
                S.irqs[i].pending = false;
                run_isr(&S.irqs[i]);
            }
        preempt_if_needed();
    }
}

/* ------------------------------------------------------------------ waiters */
static rtos_task_t *best_waiter(enum wait w, const void *obj) {
    rtos_task_t *best = NULL;
    for (int i = 0; i < S.ntasks; i++) {
        rtos_task_t *t = S.tasks[i];
        if (t->state != BLOCKED || t->wait != w || t->wait_obj != obj) continue;
        if (!best || t->prio > best->prio || (t->prio == best->prio && t->seq < best->seq)) best = t;
    }
    return best;
}

static void after_wake(void) {
    if (S.in_isr) return;
    preempt_if_needed();
}

/* ------------------------------------------------------------------ semaphores */
rtos_sem_t *rtos_sem_create(unsigned initial, unsigned max) {
    ensure_init();
    if (max == 0) sim_fail("a semaphore needs max >= 1");
    if (initial > max) sim_fail("semaphore initial count %u is above its max %u", initial, max);
    rtos_sem_t *s = sim_alloc(sizeof *s);
    s->count = initial;
    s->max = max;
    return s;
}

bool rtos_sem_take(rtos_sem_t *s, rtos_tick_t timeout) {
    if (!s) sim_fail("rtos_sem_take(NULL)");
    if (s->count > 0) {
        s->count--;
        return true;
    }
    if (timeout == RTOS_NO_WAIT) return false;
    require_task("rtos_sem_take");
    if (S.critical) sim_fail("blocking on a semaphore inside a critical section");
    ev("wait", S.current->name, "semaphore");
    block(W_SEM, s, timeout);
    return !S.current->timed_out;
}

bool rtos_sem_give(rtos_sem_t *s) {
    if (!s) sim_fail("rtos_sem_give(NULL)");
    rtos_task_t *w = best_waiter(W_SEM, s);
    if (w) {
        make_ready(w, S.in_isr); /* hand the token straight over */
        after_wake();
        return true;
    }
    if (s->count >= s->max) return false;
    s->count++;
    return true;
}

unsigned rtos_sem_count(const rtos_sem_t *s) { return s ? s->count : 0; }

/* ------------------------------------------------------------------ mutexes */
rtos_mutex_t *rtos_mutex_create(bool pi) {
    ensure_init();
    rtos_mutex_t *m = sim_alloc(sizeof *m);
    m->pi = pi;
    return m;
}

/* Effective priority = base, raised by the waiters of every PI mutex the task holds. */
static int inherited_prio(const rtos_task_t *t) {
    int p = t->base_prio;
    for (int i = 0; i < S.ntasks; i++) {
        const rtos_task_t *w = S.tasks[i];
        if (w->state == BLOCKED && w->wait == W_MUTEX) {
            const rtos_mutex_t *m = w->wait_obj;
            if (m->owner == t && m->pi && w->prio > p) p = w->prio;
        }
    }
    return p;
}

static void propagate(rtos_task_t *owner) {
    for (int depth = 0; owner && depth < MAX_TASKS; depth++) {
        int p = inherited_prio(owner);
        if (p == owner->prio) return;
        set_prio(owner, p);
        if (owner->state == BLOCKED && owner->wait == W_MUTEX) owner = ((rtos_mutex_t *)owner->wait_obj)->owner;
        else return;
    }
}

bool rtos_mutex_lock(rtos_mutex_t *m, rtos_tick_t timeout) {
    if (!m) sim_fail("rtos_mutex_lock(NULL)");
    if (S.in_isr) sim_fail("mutexes can't be used from an ISR (an ISR can't own or wait for a lock)");
    require_task("rtos_mutex_lock");
    rtos_task_t *me = S.current;
    if (m->owner == NULL) {
        m->owner = me;
        ev("lock", me->name, "");
        return true;
    }
    if (m->owner == me) sim_fail("task \"%s\" locked a mutex it already holds — that deadlocks a non-recursive mutex", me->name);
    if (timeout == RTOS_NO_WAIT) return false;
    if (S.critical) sim_fail("blocking on a mutex inside a critical section");
    ev("wait", me->name, "mutex");
    me->wait = W_MUTEX;
    me->wait_obj = m;
    me->state = BLOCKED; /* visible to propagate() before we switch away */
    if (m->pi) propagate(m->owner);
    block(W_MUTEX, m, timeout);
    if (me->timed_out) {
        if (m->owner) {
            set_prio(m->owner, inherited_prio(m->owner));
        }
        return false;
    }
    return true;
}

void rtos_mutex_unlock(rtos_mutex_t *m) {
    if (!m) sim_fail("rtos_mutex_unlock(NULL)");
    if (S.in_isr) sim_fail("mutexes can't be used from an ISR");
    rtos_task_t *me = S.current;
    if (m->owner != me)
        sim_fail("task \"%s\" unlocked a mutex it doesn't own (%s)", me ? me->name : "?", m->owner ? m->owner->name : "it isn't locked");
    ev("unlock", me->name, "");
    rtos_task_t *w = best_waiter(W_MUTEX, m);
    m->owner = w;
    set_prio(me, inherited_prio(me)); /* drop any inherited priority */
    if (w) {
        make_ready(w, false);
        ev("lock", w->name, "");
        if (m->pi) propagate(w);
    }
    preempt_if_needed();
}

rtos_task_t *rtos_mutex_owner(const rtos_mutex_t *m) { return m ? m->owner : NULL; }

/* ------------------------------------------------------------------ queues */
rtos_queue_t *rtos_queue_create(size_t length, size_t item_size) {
    ensure_init();
    if (length == 0 || item_size == 0) sim_fail("a queue needs length > 0 and item_size > 0");
    rtos_queue_t *q = sim_alloc(sizeof *q);
    q->buf = sim_alloc(length * item_size);
    q->len = length;
    q->size = item_size;
    return q;
}

void sim_queue_handoff(rtos_queue_t *q, bool on) { if (q) q->handoff = on; }

static void q_push(rtos_queue_t *q, const void *item) {
    memcpy(q->buf + ((q->head + q->count) % q->len) * q->size, item, q->size);
    q->count++;
}

static void q_pop(rtos_queue_t *q, void *out) {
    memcpy(out, q->buf + q->head * q->size, q->size);
    q->head = (q->head + 1) % q->len;
    q->count--;
}

bool rtos_queue_send(rtos_queue_t *q, const void *item, rtos_tick_t timeout) {
    if (!q || !item) sim_fail("rtos_queue_send(NULL, ...)");
    rtos_task_t *r = q->count == 0 ? best_waiter(W_QRECV, q) : NULL;
    if (r && q->handoff) {
        memcpy(r->wait_buf, item, q->size); /* straight into the waiting receiver */
        r->got_item = true;
        make_ready(r, S.in_isr);
        after_wake();
        return true;
    }
    if (q->count < q->len) {
        q_push(q, item);
        if (r) {
            make_ready(r, S.in_isr); /* it takes the item from the queue when it runs */
            after_wake();
        }
        return true;
    }
    if (timeout == RTOS_NO_WAIT) return false;
    require_task("rtos_queue_send");
    if (S.critical) sim_fail("blocking on a full queue inside a critical section");
    ev("wait", S.current->name, "queue full");
    S.current->wait_buf = (void *)item;
    block(W_QSEND, q, timeout);
    return !S.current->timed_out;
}

bool rtos_queue_receive(rtos_queue_t *q, void *out, rtos_tick_t timeout) {
    if (!q || !out) sim_fail("rtos_queue_receive(NULL, ...)");
    rtos_tick_t deadline = timeout == RTOS_WAIT_FOREVER ? RTOS_WAIT_FOREVER : S.now + timeout;
    for (;;) {
        if (q->count > 0) {
            q_pop(q, out);
            rtos_task_t *s = best_waiter(W_QSEND, q);
            if (s) {
                q_push(q, s->wait_buf);
                make_ready(s, S.in_isr);
                after_wake();
            }
            return true;
        }
        if (timeout == RTOS_NO_WAIT || (deadline != RTOS_WAIT_FOREVER && S.now >= deadline)) return false;
        require_task("rtos_queue_receive");
        if (S.critical) sim_fail("blocking on an empty queue inside a critical section");
        ev("wait", S.current->name, "queue empty");
        S.current->wait_buf = out;
        S.current->got_item = false;
        block(W_QRECV, q, deadline == RTOS_WAIT_FOREVER ? RTOS_WAIT_FOREVER : deadline - S.now);
        if (S.current->timed_out) return false;
        if (S.current->got_item) return true; /* handed over directly (Zephyr) */
        /* Woken because an item arrived: loop and take it, unless a higher-priority receiver got there first. */
    }
}

size_t rtos_queue_count(const rtos_queue_t *q) { return q ? q->count : 0; }

/* ------------------------------------------------------------------ notifications */
void rtos_notify_give(rtos_task_t *t) {
    if (!t) sim_fail("rtos_notify_give(NULL)");
    t->notify++;
    if (t->state == BLOCKED && t->wait == W_NOTIFY) {
        make_ready(t, S.in_isr);
        after_wake();
    }
}

uint32_t rtos_notify_take(bool clear_on_exit, rtos_tick_t timeout) {
    require_task("rtos_notify_take");
    rtos_task_t *me = S.current;
    if (me->notify == 0 && timeout != RTOS_NO_WAIT) {
        ev("wait", me->name, "notification");
        block(W_NOTIFY, NULL, timeout);
    }
    uint32_t v = me->notify;
    if (v) me->notify = clear_on_exit ? 0 : v - 1;
    return v;
}

/* ------------------------------------------------------------------ timers */
rtos_timer_t *rtos_timer_create(const char *name, rtos_tick_t period, bool periodic, rtos_timer_fn cb, void *arg) {
    ensure_init();
    if (!cb) sim_fail("a timer needs a callback");
    if (period == 0 && periodic) sim_fail("a periodic timer needs a period > 0");
    if (S.ntimers >= MAX_TIMERS) sim_fail("too many timers");
    rtos_timer_t *t = sim_alloc(sizeof *t);
    snprintf(t->name, sizeof t->name, "%s", name ? name : "timer");
    t->period = period;
    t->periodic = periodic;
    t->cb = cb;
    t->arg = arg;
    S.timers[S.ntimers++] = t;
    return t;
}

void rtos_timer_start(rtos_timer_t *t, rtos_tick_t first_delay) {
    if (!t) sim_fail("rtos_timer_start(NULL)");
    t->next = S.now + (first_delay ? first_delay : t->period);
    t->active = true;
}

void rtos_timer_stop(rtos_timer_t *t) {
    if (t) t->active = false;
}
bool rtos_timer_is_active(const rtos_timer_t *t) { return t && t->active; }

/* ------------------------------------------------------------------ trace export + cleanup */
static void json_str(FILE *f, const char *s) {
    fputc('"', f);
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\') fprintf(f, "\\%c", c);
        else if (c < 32) fprintf(f, "\\u%04x", c);
        else fputc(c, f);
    }
    fputc('"', f);
}

/* Called by the harness after each test (before the leak check). */
void dts_after_test(const char *test_name) {
    if (!S.init) return;
    FILE *f = fdopen(dup(4), "w");
    if (f) {
        fprintf(f, "{\"test\":");
        json_str(f, test_name);
        fprintf(f, ",\"ticks\":%u,\"tasks\":[", (unsigned)S.traced);
        for (int i = 0; i < S.ntasks; i++) {
            fprintf(f, "%s{\"name\":", i ? "," : "");
            json_str(f, S.tasks[i]->name);
            fprintf(f, ",\"prio\":%d,\"coop\":%s}", S.tasks[i]->base_prio, S.tasks[i]->coop ? "true" : "false");
        }
        fprintf(f, "],\"runs\":[");
        /* run-length encode the per-tick trace: [start, end, taskIndex|-1] */
        bool first = true;
        rtos_tick_t lim = S.traced < MAX_TRACE_TICKS ? S.traced : MAX_TRACE_TICKS;
        for (rtos_tick_t i = 0; i < lim;) {
            rtos_tick_t j = i;
            while (j < lim && S.trace[j] == S.trace[i]) j++;
            fprintf(f, "%s[%u,%u,%d]", first ? "" : ",", (unsigned)i, (unsigned)j, S.trace[i]);
            first = false;
            i = j;
        }
        fprintf(f, "],\"events\":[");
        for (int i = 0; i < S.nevents; i++) {
            fprintf(f, "%s[%u,", i ? "," : "", (unsigned)S.events[i].tick);
            json_str(f, S.events[i].type);
            fputc(',', f);
            json_str(f, S.events[i].a);
            fputc(',', f);
            json_str(f, S.events[i].b);
            fputc(']', f);
        }
        fprintf(f, "],\"deadlock\":%s}\x1e", S.deadlock_reported ? "true" : "false");
        fclose(f);
    }
    for (int i = 0; i < S.nallocs; i++) free(S.allocs[i]);
    S.nallocs = 0;
}
