/* Milestone 3: splitting, dispatching, the first commands and the main loop. */
#include <stdio.h>

#include "check.h"
#include "commands.h"
#include "line.h"
#include "shell.h"
#include "sim.h"
#include "uart.h"

static void send(const char *s) {
    while (*s) sim_uart_rx((uint8_t)*s++);
}

static void split_words(void) {
    char line[] = "led blink 3";
    char *argv[SHELL_MAX_ARGS] = { 0 };
    CHECK_INT(shell_split(line, argv, SHELL_MAX_ARGS), 3);
    CHECK_STR(argv[0], "led");
    CHECK_STR(argv[1], "blink");
    CHECK_STR(argv[2], "3");
}

static void split_extra_whitespace(void) {
    char line[] = "  gpio   set\t5  ";
    char *argv[SHELL_MAX_ARGS] = { 0 };
    CHECK_INT(shell_split(line, argv, SHELL_MAX_ARGS), 3);
    CHECK_STR(argv[0], "gpio");
    CHECK_STR(argv[1], "set");
    CHECK_STR(argv[2], "5");
}

static void split_empty(void) {
    char a[] = "", b[] = "  \t ";
    char *argv[SHELL_MAX_ARGS] = { 0 };
    CHECK_INT(shell_split(a, argv, SHELL_MAX_ARGS), 0);
    CHECK_INT(shell_split(b, argv, SHELL_MAX_ARGS), 0);
}

static void split_limit(void) {
    char a[] = "a b", b[] = "a b c";
    char *argv[2] = { 0 };
    CHECK_INT(shell_split(a, argv, 2), 2);
    CHECK_INT(shell_split(b, argv, 2), -1);
}

static void split_in_place(void) {
    char line[] = "reg read 4";
    char *argv[SHELL_MAX_ARGS] = { 0 };
    CHECK_INT(shell_split(line, argv, SHELL_MAX_ARGS), 3);
    CHECK(argv[0] == &line[0]);
    CHECK(argv[1] == &line[4]);
    CHECK(argv[2] == &line[9]);
    CHECK_INT(line[3], '\0');
}

static void table_has_the_m3_commands(void) {
    const char *want[] = { "help", "echo", "stats" };
    for (size_t w = 0; w < 3; w++) {
        int found = 0;
        for (size_t i = 0; i < shell_num_commands; i++)
            if (strcmp(shell_commands[i].name, want[w]) == 0 && shell_commands[i].fn && shell_commands[i].help) found = 1;
        if (!found) check_fail(__FILE__, __LINE__, "no \"%s\" entry in shell_commands", want[w]);
    }
}

static void exec_unknown(void) {
    sim_reset();
    CHECK(shell_exec("frobnicate now") != 0);
    CHECK_STR(sim_tx(), "unknown command: frobnicate\r\n");
}

static void exec_empty(void) {
    sim_reset();
    CHECK_INT(shell_exec("   "), 0);
    CHECK_STR(sim_tx(), "");
}

static void exec_doesnt_modify_line(void) {
    char line[] = "echo a b";
    sim_reset();
    shell_exec(line);
    CHECK_STR(line, "echo a b");
}

static void exec_too_many_words(void) {
    sim_reset();
    CHECK_INT(shell_exec("echo 1 2 3 4 5"), 0);
    CHECK_STR(sim_tx(), "1 2 3 4 5\r\n");
    sim_tx_clear();
    CHECK(shell_exec("echo 1 2 3 4 5 6") != 0);
    CHECK_STR(sim_tx(), "error: too many arguments\r\n");
}

static void exec_too_long(void) {
    sim_reset();
    CHECK(shell_exec("echo 0123456789012345678901234567890") != 0);
    CHECK_STR(sim_tx(), "error: line too long (max 32)\r\n");
}

static void echo_joins_words(void) {
    sim_reset();
    CHECK_INT(shell_exec("echo  hello   world"), 0);
    CHECK_STR(sim_tx(), "hello world\r\n");
    sim_tx_clear();
    CHECK_INT(shell_exec("echo"), 0);
    CHECK_STR(sim_tx(), "\r\n");
}

static void help_lists_the_table(void) {
    char want[1024] = "";
    for (size_t i = 0; i < shell_num_commands; i++) {
        char row[128];
        snprintf(row, sizeof row, "%-8s%s\r\n", shell_commands[i].name, shell_commands[i].help);
        strncat(want, row, sizeof want - strlen(want) - 1);
    }
    sim_reset();
    CHECK_INT(shell_exec("help"), 0);
    CHECK_STR(sim_tx(), want);
}

static void stats_shows_counters(void) {
    sim_reset();
    uart_init();
    for (int i = 0; i < 70; i++) sim_uart_rx('z');
    sim_uart_rx_masked('a');
    sim_uart_rx_masked('b');
    sim_uart_irq();
    sim_tx_clear();
    CHECK_INT(shell_exec("stats"), 0);
    CHECK_STR(sim_tx(), "rx dropped 8, overruns 1\r\n");
}

static void init_prints_banner(void) {
    sim_reset();
    shell_init();
    CHECK_STR(sim_tx(), "uart-shell ready\r\n> ");
}

static void poll_runs_a_command(void) {
    sim_reset();
    shell_init();
    sim_tx_clear();
    send("echo hi\r");
    shell_poll();
    CHECK_STR(sim_tx(), "echo hi\r\nhi\r\n> ");
    sim_tx_clear();
    send("\r");
    shell_poll();
    CHECK_STR(sim_tx(), "\r\n> ");
}

static void poll_handles_split_input(void) {
    sim_reset();
    shell_init();
    sim_tx_clear();
    send("ec");
    shell_poll();   /* half a line: nothing to run yet */
    send("ho x\r\nnope\r");
    shell_poll();
    CHECK_STR(sim_tx(), "echo x\r\nx\r\n> nope\r\nunknown command: nope\r\n> ");
}

static void poll_reports_too_long(void) {
    char in[42], want[128];
    sim_reset();
    shell_init();
    sim_tx_clear();
    memset(in, 'x', 40);
    in[40] = '\r';
    in[41] = '\0';
    send(in);
    shell_poll();
    memset(want, 'x', LINE_MAX);
    strcpy(want + LINE_MAX, "\r\nerror: line too long (max 32)\r\n> ");
    CHECK_STR(sim_tx(), want);
}

void suite_m3(void) {
    RUN(split_words, "m3: shell_split splits on spaces");
    RUN(split_extra_whitespace, "m3: shell_split skips runs of spaces and tabs");
    RUN(split_empty, "m3: shell_split of a blank line gives 0");
    RUN(split_limit, "m3: shell_split returns -1 for too many words");
    RUN(split_in_place, "m3: shell_split works in place");
    RUN(table_has_the_m3_commands, "m3: the table has help, echo and stats");
    RUN(exec_unknown, "m3: an unknown command is reported");
    RUN(exec_empty, "m3: a blank line does nothing");
    RUN(exec_doesnt_modify_line, "m3: shell_exec leaves its argument alone");
    RUN(exec_too_many_words, "m3: more than SHELL_MAX_ARGS words is an error");
    RUN(exec_too_long, "m3: shell_exec rejects a line longer than LINE_MAX");
    RUN(echo_joins_words, "m3: echo prints its words with single spaces");
    RUN(help_lists_the_table, "m3: help lists every command in table order");
    RUN(stats_shows_counters, "m3: stats shows the UART counters");
    RUN(init_prints_banner, "m3: shell_init prints the banner and prompt");
    RUN(poll_runs_a_command, "m3: shell_poll runs a typed line and prompts again");
    RUN(poll_handles_split_input, "m3: shell_poll copes with lines arriving in pieces");
    RUN(poll_reports_too_long, "m3: shell_poll reports a line that was too long");
}
