#include <string.h>

#include "commands.h"
#include "line.h"
#include "out.h"
#include "shell.h"
#include "uart.h"

int shell_split(char *line, char *argv[], int max_args) {
    (void)line, (void)argv, (void)max_args;
    return 0;       /* TODO(m3) */
}

int shell_exec(const char *line) {
    (void)line;
    return -1;      /* TODO(m3) */
}

void shell_init(void) {
    /* TODO(m3) */
}

void shell_poll(void) {
    /* TODO(m3) */
}
