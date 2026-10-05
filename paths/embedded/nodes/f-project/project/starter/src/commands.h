/* Milestones 3 and 4: the commands and the table that names them. */
#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include <stdint.h>

#include "shell.h"

extern const shell_cmd_t shell_commands[];
extern const size_t shell_num_commands;

/* Milestone 3 */
int cmd_help(int argc, char *argv[]);
int cmd_echo(int argc, char *argv[]);
int cmd_stats(int argc, char *argv[]);

/* Milestone 4 */
bool parse_u32(const char *s, uint32_t *out);
int cmd_led(int argc, char *argv[]);
int cmd_gpio(int argc, char *argv[]);
int cmd_reg(int argc, char *argv[]);

#endif
