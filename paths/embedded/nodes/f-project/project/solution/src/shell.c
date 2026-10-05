#include <string.h>

#include "commands.h"
#include "line.h"
#include "out.h"
#include "shell.h"
#include "uart.h"

static line_t line;

static int is_space(char c) { return c == ' ' || c == '\t'; }

int shell_split(char *line, char *argv[], int max_args) {
    int argc = 0;
    for (;;) {
        while (is_space(*line)) line++;
        if (*line == '\0') return argc;
        if (argc == max_args) return -1;
        argv[argc++] = line;
        while (*line && !is_space(*line)) line++;
        if (*line) *line++ = '\0';
    }
}

static void too_long(void) {
    out_printf("error: line too long (max %d)\r\n", LINE_MAX);
}

int shell_exec(const char *text) {
    char buf[LINE_MAX + 1];
    char *argv[SHELL_MAX_ARGS];
    size_t len = strlen(text);
    if (len > LINE_MAX) {
        too_long();
        return -1;
    }
    memcpy(buf, text, len + 1);

    int argc = shell_split(buf, argv, SHELL_MAX_ARGS);
    if (argc < 0) {
        out_puts("error: too many arguments\r\n");
        return -1;
    }
    if (argc == 0) return 0;
    for (size_t i = 0; i < shell_num_commands; i++)
        if (strcmp(argv[0], shell_commands[i].name) == 0) return shell_commands[i].fn(argc, argv);
    out_printf("unknown command: %s\r\n", argv[0]);
    return -1;
}

void shell_init(void) {
    uart_init();
    line_init(&line);
    out_puts("uart-shell ready\r\n" SHELL_PROMPT);
}

void shell_poll(void) {
    uint8_t byte;
    while (uart_getc(&byte)) {
        switch (line_feed(&line, (char)byte)) {
        case LINE_READY:
            shell_exec(line_get(&line));
            out_puts(SHELL_PROMPT);
            break;
        case LINE_TOO_LONG:
            too_long();
            out_puts(SHELL_PROMPT);
            break;
        case LINE_NONE:
            break;
        }
    }
}
