/* hooks.c: the kernel's error hooks. Given. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "task.h"

void vAssertCalled(const char *file, unsigned long line)
{
    fprintf(stderr, "configASSERT failed at %s:%lu\n", file, line);
    abort();
}

void vApplicationMallocFailedHook(void)
{
    fprintf(stderr, "FreeRTOS heap exhausted (configTOTAL_HEAP_SIZE)\n");
    abort();
}

/* The idle task runs when no other task can. On a microcontroller it would
 * sleep the CPU until the next interrupt; here it sleeps the thread, so the
 * simulation doesn't keep a CPU core at 100%. The next tick signal ends the
 * sleep early. */
void vApplicationIdleHook(void)
{
    usleep(15000);
}
