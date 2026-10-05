#include <string.h>

#include "commands.h"
#include "hal.h"
#include "out.h"
#include "uart.h"

const shell_cmd_t shell_commands[] = {
    { "help",  cmd_help,  "list the commands" },
    { "echo",  cmd_echo,  "print the words back" },
    { "stats", cmd_stats, "show the UART error counters" },
    /* TODO(m4): led, gpio and reg */
};
const size_t shell_num_commands = sizeof shell_commands / sizeof shell_commands[0];

/* ---- Milestone 3 ---- */

int cmd_help(int argc, char *argv[]) {
    (void)argc, (void)argv;
    return -1;      /* TODO(m3) */
}

int cmd_echo(int argc, char *argv[]) {
    (void)argc, (void)argv;
    return -1;      /* TODO(m3) */
}

int cmd_stats(int argc, char *argv[]) {
    (void)argc, (void)argv;
    return -1;      /* TODO(m3) */
}

/* ---- Milestone 4 ---- */

bool parse_u32(const char *s, uint32_t *out) {
    (void)s, (void)out;
    return false;   /* TODO(m4) */
}

int cmd_led(int argc, char *argv[]) {
    (void)argc, (void)argv;
    return -1;      /* TODO(m4) */
}

int cmd_gpio(int argc, char *argv[]) {
    (void)argc, (void)argv;
    return -1;      /* TODO(m4) */
}

int cmd_reg(int argc, char *argv[]) {
    (void)argc, (void)argv;
    return -1;      /* TODO(m4) */
}
