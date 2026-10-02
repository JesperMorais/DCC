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

/* The CAN RX interrupt: do the minimum, signal, get out. */
void can_rx_isr(void) {
    rtos_sem_give(rx_sem); /* one token per frame; the handler wakes on this very tick */
}

static void can_rx_task(void *arg) {
    (void)arg;
    for (;;) {
        rtos_sem_take(rx_sem, RTOS_WAIT_FOREVER);
        parse_frame();
    }
}

void can_start(void) {
    rx_sem = rtos_sem_create(0, MAX_BURST);           /* counting: remembers every frame of a burst */
    rtos_task_create("can_rx", can_rx_task, NULL, 3); /* above the display (2): runs as soon as a frame lands */
}
