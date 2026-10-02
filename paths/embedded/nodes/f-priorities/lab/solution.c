/* Gimbal controller for a camera drone: three periodic tasks on one CPU.
 * Higher number = more urgent. Pick the priorities so every job meets its deadline
 * (deadline = the end of its own period). */
#include <stddef.h>
#include "rtos.h"

/* Rate-monotonic: the shorter the period, the higher the priority. */
#define PRIO_MOTOR 3     /* period 5  */
#define PRIO_IMU 2       /* period 10 */
#define PRIO_TELEMETRY 1 /* period 20 */

struct task_spec {
    const char *name;
    rtos_tick_t period; /* ticks; the deadline is the end of the period */
    rtos_tick_t wcet;   /* worst-case execution time, in ticks */
    int priority;
};

const struct task_spec gimbal_tasks[] = {
    {"motor", 5, 1, PRIO_MOTOR},
    {"imu", 10, 3, PRIO_IMU},
    {"telemetry", 20, 6, PRIO_TELEMETRY},
};
const size_t gimbal_task_count = sizeof gimbal_tasks / sizeof gimbal_tasks[0];

/* Total CPU utilisation U = sum of wcet/period, as a fraction (0.5 = 50 %). */
double utilisation(const struct task_spec *set, size_t n) {
    double u = 0.0;
    for (size_t i = 0; i < n; i++) u += (double)set[i].wcet / (double)set[i].period;
    return u;
}

/* Given: one generic periodic task. Each job burns its WCET, then waits for its next release. */
static void periodic_task(void *arg) {
    const struct task_spec *t = arg;
    rtos_tick_t release = 0; /* every task is released at tick 0: the worst case ("critical instant") */
    for (;;) {
        rtos_busy(t->wcet);
        rtos_delay_until(&release, t->period);
    }
}

void gimbal_start(void) {
    for (size_t i = 0; i < gimbal_task_count; i++)
        rtos_task_create(gimbal_tasks[i].name, periodic_task, (void *)&gimbal_tasks[i], gimbal_tasks[i].priority);
}
