/* Milestone 3: splitting a line into words and dispatching to a command. */
#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>

#define SHELL_MAX_ARGS 6     /* words per line, the command name included */
#define SHELL_PROMPT   "> "

typedef int (*cmd_fn_t)(int argc, char *argv[]);   /* returns 0 on success */

typedef struct {
    const char *name;
    cmd_fn_t fn;
    const char *help;
} shell_cmd_t;

/* Splits line in place into at most max_args words separated by spaces or
 * tabs. Stores pointers into line in argv and returns how many there are, or
 * -1 if there are more than max_args. */
int shell_split(char *line, char *argv[], int max_args);

/* Runs one line: finds the command named by the first word in the table and
 * calls it. Returns the command's result, 0 for an empty line, or -1 (with a
 * message) for anything that couldn't run. line is not modified. */
int shell_exec(const char *line);

void shell_init(void);   /* starts the UART and line editor, prints the banner and prompt */
void shell_poll(void);   /* main loop: handles every byte that has arrived so far */

#endif
