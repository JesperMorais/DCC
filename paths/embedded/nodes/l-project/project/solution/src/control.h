#ifndef CONTROL_H
#define CONTROL_H

#include <stddef.h>

/* The control socket: a listening UNIX socket and its clients. Commands are
 * lines; each gets one reply line. */

#define MAX_CLIENTS 16
#define CLIENT_BUF 128

struct client {
    int fd;               /* -1 = free slot */
    char buf[CLIENT_BUF];
    size_t len;
};

struct control {
    int listen_fd;        /* -1 = no control socket configured */
    char path[108];
    struct client clients[MAX_CLIENTS];
};

/* Marks everything closed. Call it first, socket or not. */
void control_init(struct control *c);
/* Binds path, replacing a stale socket file but nothing else. 0, or -1 with errno. */
int control_open(struct control *c, const char *path);
/* Accepts one client and returns its slot, or -1 (full: the client is dropped). */
int control_accept(struct control *c);
void control_drop(struct control *c, int slot);
/* Closes everything and removes the socket file. */
void control_close(struct control *c);

#endif
