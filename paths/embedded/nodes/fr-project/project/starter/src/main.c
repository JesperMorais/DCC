/* main.c: the plant controller. Yours to write.
 *
 * Start with milestone 1. How you split the code into tasks, functions and
 * files is up to you: the Makefile compiles every .c file in src/. */
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#include "hw.h"
#include "plant.h"

int main(int argc, char **argv)
{
    hw_init(argc, argv);

    /* TODO: create the tasks and kernel objects, then start the scheduler. */
    fprintf(stderr, "not implemented yet: start with milestone 1\n");
    return 1;
}
