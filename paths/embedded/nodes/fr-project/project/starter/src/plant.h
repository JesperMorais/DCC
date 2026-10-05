/* plant.h: the plant controller's data types and configuration.
 *
 * Given. Use these types and constants; add your own headers for the rest.
 * The milestones (in the app) say what each piece must do. */
#ifndef PLANT_H
#define PLANT_H

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"

/* ---- Timing (1 tick = 1 ms) ------------------------------------------- */

#define SENSOR_PERIOD_MS    100  /* one level sample every 100 ms, drift-free */
#define WATCHDOG_PERIOD_MS  250  /* heartbeat / watchdog window */

/* ---- Control ----------------------------------------------------------- */

#define LEVEL_HIGH_MM       700  /* pump on at or above this level */
#define LEVEL_LOW_MM        300  /* pump off at or below this level */

/* ---- Queues ------------------------------------------------------------ */

#define READING_QUEUE_LEN   4
#define LOG_QUEUE_LEN       32
#define LOG_TEXT_MAX        80

/* One level sample, sent by value from the sensor task to the control task. */
typedef struct {
    TickType_t tick;     /* when the conversion was started */
    int        level_mm; /* the measured level */
} reading_t;

/* One log line, sent by value to the logger task, the only task that prints.
 * The logger prints it as "<tick> <text>". */
typedef struct {
    TickType_t tick;
    bool       last;               /* the final line: print it, then shut down */
    char       text[LOG_TEXT_MAX]; /* e.g. "SENSOR level=412" */
} log_line_t;

/* The plant's shared status. Several tasks read and write it, so every
 * access goes through one mutex. */
typedef struct {
    int      last_level_mm;
    int      max_level_mm;
    uint32_t samples;      /* readings the control task has handled */
    uint32_t pump_starts;
    bool     pump_on;
    bool     estop;        /* latched by the e-stop interrupt (milestone 3) */
    bool     fault;        /* latched by the watchdog (milestone 4) */
} plant_status_t;

#endif /* PLANT_H */
