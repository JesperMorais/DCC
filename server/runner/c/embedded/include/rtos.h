/* daily.ts RTOS simulator — a deterministic, fixed-priority preemptive kernel.
 *
 * Time is virtual: 1 tick = 1 ms. Code between RTOS calls takes zero time; only
 * rtos_busy(n) (CPU work), delays and waiting make time pass. That makes every run
 * reproducible, so tests can grade *timing* and the UI can draw your timeline.
 *
 * Priorities: a HIGHER number is MORE urgent (like FreeRTOS). Equal priorities
 * share the CPU round-robin, one tick at a time.
 */
#ifndef DTS_RTOS_H
#define DTS_RTOS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint32_t rtos_tick_t;
#define RTOS_NO_WAIT ((rtos_tick_t)0)
#define RTOS_WAIT_FOREVER ((rtos_tick_t)0xFFFFFFFFu)

typedef struct rtos_task rtos_task_t;
typedef struct rtos_sem rtos_sem_t;
typedef struct rtos_mutex rtos_mutex_t;
typedef struct rtos_queue rtos_queue_t;
typedef struct rtos_timer rtos_timer_t;
typedef void (*rtos_task_fn)(void *arg);
typedef void (*rtos_timer_fn)(void *arg);

/* ---- tasks ---- */
rtos_task_t *rtos_task_create(const char *name, rtos_task_fn fn, void *arg, int priority);
void rtos_delay(rtos_tick_t ticks);                               /* block for ticks */
void rtos_delay_until(rtos_tick_t *last_wake, rtos_tick_t period); /* drift-free periodic wait */
void rtos_yield(void);
void rtos_busy(rtos_tick_t ticks);                                /* burn CPU: simulated work, preemptible */
rtos_tick_t rtos_now(void);
rtos_task_t *rtos_self(void);
const char *rtos_task_name(const rtos_task_t *t);
int rtos_task_priority(const rtos_task_t *t);                     /* effective (incl. inheritance) */
void rtos_task_set_priority(rtos_task_t *t, int priority);
void rtos_task_exit(void);                                        /* end the calling task */

/* ---- critical sections: no preemption, interrupts held off until exit ---- */
void rtos_enter_critical(void);
void rtos_exit_critical(void);

/* ---- semaphores (give is ISR-safe) ---- */
rtos_sem_t *rtos_sem_create(unsigned initial, unsigned max);
bool rtos_sem_take(rtos_sem_t *s, rtos_tick_t timeout);
bool rtos_sem_give(rtos_sem_t *s);
unsigned rtos_sem_count(const rtos_sem_t *s);

/* ---- mutexes (task context only) ---- */
rtos_mutex_t *rtos_mutex_create(bool priority_inheritance);
bool rtos_mutex_lock(rtos_mutex_t *m, rtos_tick_t timeout);
void rtos_mutex_unlock(rtos_mutex_t *m);
rtos_task_t *rtos_mutex_owner(const rtos_mutex_t *m);

/* ---- queues: fixed-size items copied in and out (send is ISR-safe with RTOS_NO_WAIT) ---- */
rtos_queue_t *rtos_queue_create(size_t length, size_t item_size);
bool rtos_queue_send(rtos_queue_t *q, const void *item, rtos_tick_t timeout);
bool rtos_queue_receive(rtos_queue_t *q, void *out, rtos_tick_t timeout);
size_t rtos_queue_count(const rtos_queue_t *q);

/* ---- direct-to-task notifications (give is ISR-safe) ---- */
void rtos_notify_give(rtos_task_t *t);
uint32_t rtos_notify_take(bool clear_on_exit, rtos_tick_t timeout);

/* ---- timers: the callback runs in INTERRUPT context ---- */
rtos_timer_t *rtos_timer_create(const char *name, rtos_tick_t period, bool periodic, rtos_timer_fn cb, void *arg);
void rtos_timer_start(rtos_timer_t *t, rtos_tick_t first_delay);
void rtos_timer_stop(rtos_timer_t *t);
bool rtos_timer_is_active(const rtos_timer_t *t);

#endif
