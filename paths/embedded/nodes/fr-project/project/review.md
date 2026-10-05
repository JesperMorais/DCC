# How we'd structure it

```
src/
  app.h        priorities, shared handles, notification bits, module entry points
  log.c        the logger task and log_at / log_now / log_last
  plant.c      sensor, control and safety tasks, and the e-stop ISR
  watchdog.c   the timer and the check-in counter
  main.c       create everything, the supervisor that ends the run
```

Priorities, from most to least urgent: **safety 7**, Tmr Svc 6, control 5, sensor 4, supervisor 2, logger 1. Safety sits above the timer daemon for the reason the boss taught: nothing should be able to delay making the plant safe. The logger is the lowest, because printing is the least urgent thing the system does.

## Decision 1: one task owns stdout

On the POSIX port this is a correctness rule, not just tidiness. `printf` takes a pthread lock inside glibc. The kernel can switch tasks at any tick, including while a task holds that lock, and the next task to print then blocks on a lock that FreeRTOS doesn't know about. So every line is a `log_line_t` sent by value:

```c
bool log_at(TickType_t tick, const char *fmt, ...);  /* never blocks */
void log_last(const char *fmt, ...);                 /* waits for room, marks the end */
```

Ordinary lines use a zero timeout, which makes them safe from the timer callback. A full queue drops the line rather than stall a control task. The final line waits, and when the logger sees `last` it flushes and calls `vTaskEndScheduler()`. On this port that stops the tick thread and makes `vTaskStartScheduler()` return in `main`, so the process exits normally and the board's report gets printed.

## Decision 2: decide and act under the same lock

Control checks the latches and switches the pump in **one** locked section:

```c
xSemaphoreTake(g_status_mutex, portMAX_DELAY);
/* update counters; want = latched ? false : hysteresis(...) */
if (want != s->pump_on) { s->pump_on = want; hw_pump_set(want); }
xSemaphoreGive(g_status_mutex);
```

If control released the mutex between deciding "on" and calling `hw_pump_set(true)`, the safety task could latch the e-stop in that gap, and control would then switch the pump on *after* the stop. Holding the lock across one fast hardware write is fine. If control holds it when safety wants it, priority inheritance lifts control to 7 for those few microseconds. Logging stays outside the lock.

## Decision 3: the ISR only signals

```c
if (estop_irqs++ == 0) estop_first_irq_tick = xTaskGetTickCountFromISR();
xTaskNotifyFromISR(g_safety_task, SHUTDOWN_ESTOP, eSetBits, &woken);
portYIELD_FROM_ISR(woken);
```

The bounce counter is `volatile` and only the ISR writes it, so it needs no lock (an ISR can't take a mutex anyway). Using notification **bits** instead of a count lets one safety task handle both reasons to shut down, the e-stop and the watchdog, with one `xTaskNotifyWait`. Three bouncing edges still produce one ESTOP line, because the task logs only on the transition into the latched state. Without `portYIELD_FROM_ISR`, the ISR returns to the idle task and the pump runs until the next tick. That's what the board's `estop_preempted` check catches.

## Decision 4: the watchdog watches the work, not the CPU

The sensor checks in after a **completed** sample. The callback reads and clears the counter inside `taskENTER_CRITICAL()`, which on this port blocks signals, the same thing "interrupts off" means on a Cortex-M. A timer that merely fired would prove that the daemon runs, not that the plant is controlled. The trip doesn't take the mutex from the callback, since that could block the daemon. It sends a notification bit, and the safety task does the locked part.

## Smaller things worth noticing

- `xTaskDelayUntil` keeps the wake time, and the SENSOR line is stamped with it, not with "now". That's why the stamps are exactly 100 apart even when a higher-priority task delays the sensor by a tick.
- `READING_QUEUE_LEN` is 4 and the sensor sends with a zero timeout. A stuck control task costs readings, not the sensor's timing.
- On real hardware you'd add a hardware watchdog (IWDG) that only the safety path kicks, and `configCHECK_FOR_STACK_OVERFLOW`, which is meaningless on pthread stacks.

Where next? Swap `hw.c` for a real board: the same `main.c`, the Cortex-M4 port and an ADC driver. That's the point of the POSIX port: you can test logic on a PC and keep the kernel identical.
