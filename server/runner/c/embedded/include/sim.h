/* daily.ts RTOS simulator — controls and introspection for TESTS.
 * (Learners usually don't need this; tests use it to drive time and grade timing.) */
#ifndef DTS_SIM_H
#define DTS_SIM_H

#include "rtos.h"

/* Run the scheduler for `ticks` of virtual time, then return to the test. Can be called repeatedly. */
void sim_run(rtos_tick_t ticks);

/* Hardware interrupts, raised at exact ticks. The handler runs in ISR context. */
void sim_irq_at(rtos_tick_t tick, const char *name, void (*isr)(void));
void sim_irq_every(rtos_tick_t first, rtos_tick_t period, const char *name, void (*isr)(void));
bool sim_in_isr(void);

/* Introspection */
rtos_tick_t sim_ran_ticks(const char *task_name);           /* CPU ticks consumed by a task */
const char *sim_running_at(rtos_tick_t tick);               /* task name, or "idle" */
rtos_tick_t sim_first_run_at_or_after(const char *task_name, rtos_tick_t tick); /* RTOS_WAIT_FOREVER if never */
int sim_context_switches(void);
bool sim_deadlocked(void);
bool sim_task_finished(const char *task_name);
int sim_max_priority_seen(const char *task_name);            /* highest effective priority (inheritance shows here) */
void sim_mark(const char *label);                            /* annotate the timeline */

/* Equal-priority tasks share the CPU tick by tick (default true). */
void sim_set_time_slicing(bool on);
void sim_set_time_slice(rtos_tick_t ticks); /* round-robin quantum for equal priorities (default 1 tick; Zephyr 20) */

/* ---- for API facades (FreeRTOS / Zephyr) ---- */
rtos_task_t *sim_task_create_ex(const char *name, rtos_task_fn fn, void *arg, int priority, bool cooperative, rtos_tick_t start_delay);
void sim_isr_yield(void);          /* portYIELD_FROM_ISR(pdTRUE): switch right after this ISR */
bool sim_isr_woke_higher(void);
void sim_queue_handoff(rtos_queue_t *q, bool on); /* Zephyr msgq semantics: copy straight into a waiting receiver */   /* FromISR helpers: did this ISR wake a task above the interrupted one? */
void sim_set_isr_auto_yield(bool on); /* neutral API: true. FreeRTOS: wake-ups from ISRs wait for the next tick unless yielded */
void sim_on_start(void (*fn)(void)); /* run once, before the first scheduled tick */
void *sim_calloc(size_t size); /* kernel-owned memory, freed after each test */
void sim_fail(const char *fmt, ...) __attribute__((format(printf, 1, 2), noreturn));

#endif
