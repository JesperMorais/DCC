#include "control.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

void control_init(struct control *c) {
    c->listen_fd = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) c->clients[i].fd = -1;
}

int control_open(struct control *c, const char *path) {
    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    if (strlen(path) >= sizeof addr.sun_path) {
        errno = ENAMETOOLONG;
        return -1;
    }
    strcpy(addr.sun_path, path);

    /* A socket file left by a crashed run is stale: replace it. Anything else
     * at that path is somebody's file, so refuse. */
    struct stat st;
    if (lstat(path, &st) == 0) {
        if (!S_ISSOCK(st.st_mode)) {
            errno = EEXIST;
            return -1;
        }
        unlink(path);
    }

    int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd < 0) return -1;
    if (bind(fd, (struct sockaddr *)&addr, sizeof addr) != 0 || listen(fd, 8) != 0) {
        int saved = errno;
        close(fd);
        errno = saved;
        return -1;
    }
    c->listen_fd = fd;
    snprintf(c->path, sizeof c->path, "%s", path);
    return 0;
}

int control_accept(struct control *c) {
    int fd = accept4(c->listen_fd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (fd < 0) return -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (c->clients[i].fd < 0) {
            c->clients[i] = (struct client){ .fd = fd };
            return i;
        }
    }
    close(fd);
    return -1;
}

void control_drop(struct control *c, int slot) {
    close(c->clients[slot].fd);
    c->clients[slot].fd = -1;
}

void control_close(struct control *c) {
    if (c->listen_fd < 0) return;
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (c->clients[i].fd >= 0) control_drop(c, i);
    close(c->listen_fd);
    unlink(c->path);
    c->listen_fd = -1;
}
