/* hw.h: the simulated board. Given: treat it like a datasheet, don't edit hw.c.
 *
 * The board is a sump: water flows in all the time, and a pump drains it.
 *
 *   inflow   +1 mm per ms, always
 *   pump     -2 mm per ms while on (so the level falls 1 mm/ms net)
 *   the tank overflows at 1000 mm; the pump runs dry at 0 mm
 *   the level starts at 400 mm
 *
 * Scenario options are passed on the command line and parsed by hw_init():
 *
 *   --run-ms N          how long the run should last, in ticks (default 3000)
 *   --estop-at T        the e-stop button is pressed at tick T (and bounces)
 *   --sensor-stall-at T from tick T on, the level sensor never answers
 *
 * When the process exits, the board prints its own measurements to stderr
 * ("hw: ..."), which the tests use as the ground truth. End the program with
 * exit() or by returning from main() so that line gets printed.
 */
#ifndef HW_H
#define HW_H

#include <stdbool.h>

/* Parse the scenario options and set up the simulated hardware. Call it
 * first thing in main(), before creating any FreeRTOS object. Unknown options
 * print a usage message and exit. */
void hw_init(int argc, char **argv);

/* The --run-ms value. */
int hw_run_ms(void);

/* Sample the level sensor. Starts an ADC conversion and blocks the calling
 * task until it completes, which takes 2 to 6 ms. Returns the level in mm.
 * Task context only: never call it from an ISR or a timer callback. */
int hw_read_level(void);

/* Switch the pump. Safe from any task, including the timer daemon. */
void hw_pump_set(bool on);

/* The e-stop button's current state: true while it's pressed. Once pressed
 * it stays pressed for the rest of the run. Safe from tasks and ISRs. */
bool hw_estop_pressed(void);

/* Attach your interrupt service routine to the e-stop button's interrupt.
 * It is called in interrupt context once per (bouncy) edge. Attach it before
 * vTaskStartScheduler(). Inside it, only FromISR APIs are allowed. */
void hw_estop_attach_isr(void (*isr)(void));

#endif /* HW_H */
