/* The PC "board" (given): every byte you type becomes a UART interrupt, then
 * the main loop gets a turn. On a terminal it switches to raw mode, so you get
 * your own echo, backspace and Enter (as '\r'), just like over a serial cable.
 * Piped input works too: printf 'led on\nled\n' | ./build/uart-shell */
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "shell.h"
#include "sim.h"

int main(void) {
    struct termios saved;
    int raw = isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &saved) == 0;
    if (raw) {
        struct termios t = saved;
        t.c_lflag &= ~(tcflag_t)(ICANON | ECHO | ISIG);
        t.c_iflag &= ~(tcflag_t)(ICRNL | IXON);
        t.c_cc[VMIN] = 1;
        t.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
        printf("(raw terminal: Ctrl-C or Ctrl-D quits)\r\n");
    }

    sim_reset();
    sim_stdout(true);
    shell_init();
    fflush(stdout);

    int c;
    while ((c = getchar()) != EOF) {
        if (raw && (c == 0x03 || c == 0x04)) break;
        sim_uart_rx((uint8_t)c);   /* the "interrupt" */
        shell_poll();              /* the main loop */
        fflush(stdout);
    }

    if (raw) tcsetattr(STDIN_FILENO, TCSANOW, &saved);
    printf("\r\n");
    return 0;
}
