#ifndef REPORTER_H
#define REPORTER_H

#include <stdint.h>

#define REPORTER_MIN_MS 10
#define REPORTER_MAX_MS 60000

/* Restarts the report timer with a new period. -EINVAL if it's out of range. */
int reporter_set_period(uint32_t period_ms);

#endif /* REPORTER_H */
