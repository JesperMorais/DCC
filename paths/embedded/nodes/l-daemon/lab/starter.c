#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

/* The tests read lines and hups. Add fields freely. */
struct plumb {
    int ep;                 /* epoll instance */
    int sig;                /* signalfd for SIGTERM, SIGINT and SIGHUP */
    int fifo;               /* read end of the FIFO */
    int listener;           /* listening UNIX stream socket */
    char fifo_path[108];
    char sock_path[108];
    int lines;              /* newlines read from the FIFO */
    int hups;               /* SIGHUPs read from the signalfd */
};

int plumb_open(struct plumb *p, const char *fifo_path, const char *sock_path) {
    /* TODO: block the signals, then signalfd, epoll, the FIFO and the socket */
    *p = (struct plumb){ .ep = -1, .sig = -1, .fifo = -1, .listener = -1 };
    (void)fifo_path;
    (void)sock_path;
    return -ENOSYS;
}

int plumb_run(struct plumb *p) {
    (void)p;
    return -ENOSYS;
}

void plumb_close(struct plumb *p) {
    (void)p;
}

int plumb_main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return 1;
}
