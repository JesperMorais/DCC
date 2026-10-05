/* Zephyr kernel API facade over the daily.ts RTOS simulator.
 *
 * Zephyr priorities: a LOWER number is MORE urgent. Negative priorities are COOPERATIVE:
 * once running, a cooperative thread is never preempted by another thread until it
 * sleeps, waits or yields. 0..14 are preemptible. 1 tick = 1 ms. */
#ifndef DTS_ZEPHYR_KERNEL_H
#define DTS_ZEPHYR_KERNEL_H

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "../rtos.h"
#include "../sim.h"

#define CONFIG_SYS_CLOCK_TICKS_PER_SEC 1000
#define CONFIG_NUM_COOP_PRIORITIES 16
#define CONFIG_NUM_PREEMPT_PRIORITIES 15
#define CONFIG_SYSTEM_WORKQUEUE_PRIORITY (-1)
#define K_PRIO_COOP(x) (-(CONFIG_NUM_COOP_PRIORITIES - (x)))
#define K_PRIO_PREEMPT(x) (x)
#define K_LOWEST_APPLICATION_THREAD_PRIO (CONFIG_NUM_PREEMPT_PRIORITIES - 1)

typedef struct { int64_t ticks; } k_timeout_t;
#define K_NO_WAIT ((k_timeout_t){0})
#define K_FOREVER ((k_timeout_t){-1})
#define K_TICKS(t) ((k_timeout_t){(int64_t)(t)})
#define K_MSEC(ms) ((k_timeout_t){(int64_t)(ms)})
#define K_SECONDS(s) ((k_timeout_t){(int64_t)(s) * 1000})
/* Absolute timeouts (encoded as in Zephyr: -2 - tick). No extra tick: the drift-free way to sleep until a deadline. */
#define K_TIMEOUT_ABS_TICKS(t) ((k_timeout_t){-2 - (int64_t)(t)})
#define K_TIMEOUT_ABS_MS(t) K_TIMEOUT_ABS_TICKS(t)
#define K_TIMEOUT_EQ(a, b) ((a).ticks == (b).ticks)

#define printk printf

/* ---- threads ---- */
typedef void (*k_thread_entry_t)(void *p1, void *p2, void *p3);
struct k_thread {
    rtos_task_t *task;
    k_thread_entry_t entry;
    void *p1, *p2, *p3;
};
typedef struct k_thread *k_tid_t;

#define K_THREAD_STACK_DEFINE(name, size) static char name[size]
#define K_KERNEL_STACK_DEFINE(name, size) static char name[size]
#define K_THREAD_STACK_SIZEOF(sym) sizeof(sym)
#define K_THREAD_STACK_MEMBER(sym, size) char sym[size]

k_tid_t k_thread_create(struct k_thread *thread, void *stack, size_t stack_size, k_thread_entry_t entry, void *p1, void *p2, void *p3,
                        int prio, uint32_t options, k_timeout_t delay);
void k_thread_name_set(k_tid_t thread, const char *name);
k_tid_t k_current_get(void);
int k_thread_priority_get(k_tid_t thread);
void k_thread_priority_set(k_tid_t thread, int prio);

/* Static threads, started when the "kernel boots" (the first sim_run). */
void dts_z_register_static(struct k_thread *t, const char *name, k_thread_entry_t entry, void *p1, void *p2, void *p3, int prio, int delay_ms);
#define K_THREAD_DEFINE(name, stack_size, entry, p1, p2, p3, prio, options, delay)                                 \
    static struct k_thread dts_z_thread_##name;                                                                   \
    const k_tid_t name = &dts_z_thread_##name;                                                                    \
    __attribute__((constructor)) static void dts_z_reg_##name(void) {                                             \
        dts_z_register_static(&dts_z_thread_##name, #name, (k_thread_entry_t)(entry), (void *)(p1), (void *)(p2), \
                              (void *)(p3), (prio), (delay));                                                     \
    }

int32_t k_sleep(k_timeout_t timeout); /* returns 0 */
int32_t k_msleep(int32_t ms);
void k_yield(void);
int64_t k_uptime_get(void);
uint32_t k_uptime_get_32(void);
void k_busy_wait(uint32_t usec); /* burns CPU, rounded up to whole ms in the simulator */
static inline bool k_is_in_isr(void) { return sim_in_isr(); }

/* ---- semaphores ---- */
struct k_sem { rtos_sem_t *s; unsigned init, limit; };
#define K_SEM_DEFINE(name, initial, max) struct k_sem name = {NULL, (initial), (max)}
#define K_SEM_MAX_LIMIT 0xFFFFu
int k_sem_init(struct k_sem *sem, unsigned initial, unsigned limit);
int k_sem_take(struct k_sem *sem, k_timeout_t timeout); /* 0, -EBUSY (K_NO_WAIT), -EAGAIN (timeout) */
void k_sem_give(struct k_sem *sem);                      /* ISR-safe */
unsigned k_sem_count_get(struct k_sem *sem);
void k_sem_reset(struct k_sem *sem);

/* ---- mutexes: recursive, with priority inheritance ---- */
struct k_mutex { rtos_mutex_t *m; unsigned lock_count; };
#define K_MUTEX_DEFINE(name) struct k_mutex name = {NULL, 0}
int k_mutex_init(struct k_mutex *mutex);
int k_mutex_lock(struct k_mutex *mutex, k_timeout_t timeout); /* 0, -EBUSY, -EAGAIN */
int k_mutex_unlock(struct k_mutex *mutex);                    /* 0, -EPERM (not owner), -EINVAL (not locked) */

/* ---- message queues ---- */
struct k_msgq { rtos_queue_t *q; size_t msg_size; uint32_t max_msgs; };
#define K_MSGQ_DEFINE(name, msg_size, max_msgs, align) struct k_msgq name = {NULL, (msg_size), (max_msgs)}
void k_msgq_init(struct k_msgq *q, char *buffer, size_t msg_size, uint32_t max_msgs);
int k_msgq_put(struct k_msgq *q, const void *data, k_timeout_t timeout); /* 0, -ENOMSG (full, no wait), -EAGAIN */
int k_msgq_get(struct k_msgq *q, void *data, k_timeout_t timeout);       /* 0, -ENOMSG (empty, no wait), -EAGAIN */
uint32_t k_msgq_num_used_get(struct k_msgq *q);
uint32_t k_msgq_num_free_get(struct k_msgq *q);

/* ---- work queue: items run one at a time on the system workqueue thread (prio -1, cooperative) ---- */
struct k_work;
typedef void (*k_work_handler_t)(struct k_work *work);
struct k_work { k_work_handler_t handler; bool pending; bool running; };
void dts_z_work_used(void);
#define K_WORK_DEFINE(name, handler)                                                        \
    struct k_work name = {(handler), false, false};                                                \
    __attribute__((constructor)) static void dts_z_work_##name(void) { dts_z_work_used(); }
void k_work_init(struct k_work *work, k_work_handler_t handler);
int k_work_submit(struct k_work *work); /* 1 = queued, 2 = queued while its handler runs, 0 = already pending. ISR-safe */
bool k_work_is_pending(const struct k_work *work);

/* ---- timers: expiry runs in ISR context ---- */
struct k_timer;
typedef void (*k_timer_expiry_t)(struct k_timer *timer);
typedef void (*k_timer_stop_t)(struct k_timer *timer);
struct k_timer { rtos_timer_t *t; k_timer_expiry_t expiry; k_timer_stop_t stop; uint32_t status; void *user_data; };
#define K_TIMER_DEFINE(name, expiry_fn, stop_fn) struct k_timer name = {NULL, (expiry_fn), (stop_fn), 0, NULL}
void k_timer_init(struct k_timer *timer, k_timer_expiry_t expiry, k_timer_stop_t stop);
void k_timer_start(struct k_timer *timer, k_timeout_t duration, k_timeout_t period);
void k_timer_stop(struct k_timer *timer);
uint32_t k_timer_status_get(struct k_timer *timer);
static inline void k_timer_user_data_set(struct k_timer *t, void *d) { t->user_data = d; }
static inline void *k_timer_user_data_get(const struct k_timer *t) { return t->user_data; }

#define CONTAINER_OF(ptr, type, field) ((type *)(void *)((char *)(ptr) - offsetof(type, field)))

#endif
