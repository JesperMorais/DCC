/* A bike computer. Two tasks share the GPS receiver and the SD card:
 *   nav     (prio 2): read a GPS fix, then append it to the track file on the SD card.
 *   archive (prio 1): open the archive file on the SD card, then stamp it with the GPS time.
 * Each does ROUNDS rounds and then finishes.
 *
 * Lock order (global, documented, never broken): gps_lock BEFORE sd_lock. */
#include <stdbool.h>
#include "rtos.h"

#define ROUNDS 5

rtos_mutex_t *gps_lock;
rtos_mutex_t *sd_lock;
unsigned fixes_logged;   /* rounds completed by nav */
unsigned files_archived; /* rounds completed by archive */
rtos_tick_t nav_start_delay = 1; /* when nav first wakes up (the tests try several) */

static void nav_task(void *arg) {
    (void)arg;
    rtos_delay(nav_start_delay);
    for (int i = 0; i < ROUNDS; i++) {
        rtos_mutex_lock(gps_lock, RTOS_WAIT_FOREVER);
        rtos_busy(2); /* read the fix */
        rtos_mutex_lock(sd_lock, RTOS_WAIT_FOREVER);
        rtos_busy(1); /* append it to the track */
        rtos_mutex_unlock(sd_lock);
        rtos_mutex_unlock(gps_lock);
        fixes_logged++;
        rtos_delay(3);
    }
}

static void archive_task(void *arg) {
    (void)arg;
    for (int i = 0; i < ROUNDS; i++) {
        /* Needs both, so take them in the global order: gps, then sd. */
        rtos_mutex_lock(gps_lock, RTOS_WAIT_FOREVER);
        rtos_mutex_lock(sd_lock, RTOS_WAIT_FOREVER);
        rtos_busy(2); /* open the archive file */
        rtos_busy(1); /* stamp it with the GPS time */
        rtos_mutex_unlock(sd_lock);
        rtos_mutex_unlock(gps_lock);
        files_archived++;
        rtos_delay(2);
    }
}

void bike_start(void) {
    gps_lock = rtos_mutex_create(true);
    sd_lock = rtos_mutex_create(true);
    rtos_task_create("nav", nav_task, NULL, 2);
    rtos_task_create("archive", archive_task, NULL, 1);
}
