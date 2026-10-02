/* A Mars-rover-ish flight computer. Three tasks share one SPI bus:
 *   attitude  (prio 3) every 10 ticks from tick 2: read the gyro over the bus (1), integrate (1).
 *                      Deadline: done within ATTITUDE_DEADLINE ticks of its release.
 *   compress  (prio 2) every 50 ticks from tick 6: 10 ticks of image compression. No bus.
 *   telemetry (prio 1) every 20 ticks from tick 0: format a packet (3), send it over the bus (2). */
#include <stdbool.h>
#include "rtos.h"

#define ATTITUDE_DEADLINE 4

rtos_mutex_t *spi_bus;
unsigned packets_sent;

/* Provided by the board (the tests): a bus transaction. You must own spi_bus when you call it. */
void spi_transfer(rtos_tick_t ticks);
/* Provided by the tests: the attitude job released at `release` is done. */
void attitude_done(rtos_tick_t release);

static void attitude_task(void *arg) {
    (void)arg;
    rtos_delay(2);
    rtos_tick_t release = rtos_now();
    for (;;) {
        rtos_mutex_lock(spi_bus, RTOS_WAIT_FOREVER);
        spi_transfer(1); /* read the gyro */
        rtos_mutex_unlock(spi_bus);
        rtos_busy(1); /* integrate */
        attitude_done(release);
        rtos_delay_until(&release, 10);
    }
}

static void compress_task(void *arg) {
    (void)arg;
    rtos_delay(6);
    rtos_tick_t release = rtos_now();
    for (;;) {
        rtos_busy(10);
        rtos_delay_until(&release, 50);
    }
}

static void telemetry_task(void *arg) {
    (void)arg;
    rtos_tick_t release = rtos_now();
    for (;;) {
        rtos_busy(3); /* format the packet: no bus needed, so do it OUTSIDE the lock */
        rtos_mutex_lock(spi_bus, RTOS_WAIT_FOREVER);
        spi_transfer(2); /* send it: the only part that needs the bus */
        rtos_mutex_unlock(spi_bus);
        packets_sent++;
        rtos_delay_until(&release, 20);
    }
}

void rover_start(void) {
    spi_bus = rtos_mutex_create(true); /* priority inheritance: the Pathfinder fix */
    rtos_task_create("attitude", attitude_task, NULL, 3);
    rtos_task_create("compress", compress_task, NULL, 2);
    rtos_task_create("telemetry", telemetry_task, NULL, 1);
}
