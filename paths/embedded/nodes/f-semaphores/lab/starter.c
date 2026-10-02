/* CAN receive path of a battery-management system.
 * Every CAN frame raises an interrupt. Parsing a frame takes 2 ticks of CPU. */
#include "rtos.h"

#define FRAME_COST 2 /* ticks of CPU to parse one frame */
#define MAX_BURST 16 /* the CAN controller can deliver up to 16 frames back-to-back */

rtos_sem_t *rx_sem;     /* ISR -> task signal */
unsigned frames_handled; /* frames fully parsed */

/* Given: the expensive part. Task context only (it burns CPU). */
void parse_frame(void) {
    rtos_busy(FRAME_COST);
    frames_handled++;
}

/* The CAN RX interrupt. */
void can_rx_isr(void) {
    parse_frame(); /* TODO: an ISR must not do this. Signal the handler task instead. */
}

static void can_rx_task(void *arg) {
    (void)arg;
    for (;;) {
        rtos_sem_take(rx_sem, RTOS_WAIT_FOREVER);
        parse_frame();
    }
}

void can_start(void) {
    rx_sem = rtos_sem_create(0, 1);                    /* TODO: is one slot enough for a burst? */
    rtos_task_create("can_rx", can_rx_task, NULL, 1); /* TODO: the display task runs at priority 2 */
}
