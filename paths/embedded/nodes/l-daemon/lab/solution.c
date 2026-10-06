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

enum { TAG_SIGNAL = 1, TAG_FIFO, TAG_LISTEN };

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

static int watch(struct plumb *p, int fd, uint64_t tag) {
    struct epoll_event ev = { .events = EPOLLIN, .data.u64 = tag };
    return epoll_ctl(p->ep, EPOLL_CTL_ADD, fd, &ev);
}

static int open_fifo(const char *path) {
    /* O_NONBLOCK: don't wait in open() for a writer to show up. */
    return open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
}

static int make_fifo(const char *path) {
    struct stat st;
    if (lstat(path, &st) == 0) return S_ISFIFO(st.st_mode) ? 0 : -EEXIST;
    if (errno != ENOENT) return -errno;
    return mkfifo(path, 0660) == 0 ? 0 : -errno;
}

static int make_listener(const char *path) {
    struct stat st;
    if (lstat(path, &st) == 0) {
        if (!S_ISSOCK(st.st_mode)) return -EEXIST;   /* not ours to delete */
        if (unlink(path) != 0) return -errno;        /* a stale socket from a crash */
    } else if (errno != ENOENT) {
        return -errno;
    }
    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    if (strlen(path) >= sizeof addr.sun_path) return -ENAMETOOLONG;
    strcpy(addr.sun_path, path);
    int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd < 0) return -errno;
    if (bind(fd, (struct sockaddr *)&addr, sizeof addr) != 0 || listen(fd, 8) != 0) {
        int err = -errno;
        close(fd);
        return err;
    }
    return fd;
}

void plumb_close(struct plumb *p);

int plumb_open(struct plumb *p, const char *fifo_path, const char *sock_path) {
    *p = (struct plumb){ .ep = -1, .sig = -1, .fifo = -1, .listener = -1 };
    if (strlen(fifo_path) >= sizeof p->fifo_path || strlen(sock_path) >= sizeof p->sock_path) return -ENAMETOOLONG;
    strcpy(p->fifo_path, fifo_path);

    /* Block first: from here on these signals wait for us instead of killing us. */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGHUP);
    int err = 0;
    if (sigprocmask(SIG_BLOCK, &set, NULL) != 0) return -errno;
    if ((p->ep = epoll_create1(EPOLL_CLOEXEC)) < 0 ||
        (p->sig = signalfd(-1, &set, SFD_NONBLOCK | SFD_CLOEXEC)) < 0) {
        err = -errno;
        goto fail;
    }
    if ((err = make_fifo(fifo_path)) != 0) goto fail;
    if ((p->fifo = open_fifo(fifo_path)) < 0) {
        err = -errno;
        goto fail;
    }
    if ((p->listener = make_listener(sock_path)) < 0) {
        err = p->listener;
        goto fail;
    }
    strcpy(p->sock_path, sock_path);   /* only now is the socket file ours to remove */
    if (watch(p, p->sig, TAG_SIGNAL) != 0 || watch(p, p->fifo, TAG_FIFO) != 0 ||
        watch(p, p->listener, TAG_LISTEN) != 0) {
        err = -errno;
        goto fail;
    }
    return 0;
fail:
    plumb_close(p);
    return err;
}

/* Reads what's there. At EOF (the last writer left), swaps in a fresh read end:
 * the new one first, then the old one goes, so there's never a moment with no
 * reader. A writer arriving in such a moment would get ENXIO or EPIPE. */
static int on_fifo(struct plumb *p) {
    char buf[512];
    for (;;) {
        ssize_t n = read(p->fifo, buf, sizeof buf);
        if (n > 0) {
            for (ssize_t i = 0; i < n; i++) p->lines += buf[i] == '\n';
        } else if (n < 0 && errno == EINTR) {
            continue;
        } else if (n < 0 && errno == EAGAIN) {
            return 0;
        } else {
            break;
        }
    }
    int fd = open_fifo(p->fifo_path);
    if (fd < 0 || watch(p, fd, TAG_FIFO) != 0) {
        int err = -errno;
        if (fd >= 0) close(fd);
        return err;
    }
    epoll_ctl(p->ep, EPOLL_CTL_DEL, p->fifo, NULL);
    close(p->fifo);
    p->fifo = fd;
    return 0;
}

static void on_client(struct plumb *p) {
    int c;
    while ((c = accept4(p->listener, NULL, NULL, SOCK_CLOEXEC)) >= 0) {
        char reply[64];
        int len = snprintf(reply, sizeof reply, "lines=%d hups=%d\n", p->lines, p->hups);
        if (write(c, reply, (size_t)len) != len) { /* the client left early: its loss */ }
        close(c);
    }
}

static bool on_signal(struct plumb *p) {
    struct signalfd_siginfo si;
    bool stop = false;
    while (read(p->sig, &si, sizeof si) == sizeof si) {
        if (si.ssi_signo == SIGHUP) p->hups++;
        else stop = true;
    }
    return stop;
}

int plumb_run(struct plumb *p) {
    struct epoll_event evs[8];
    bool stopping = false;
    for (;;) {
        int n = epoll_wait(p->ep, evs, 8, stopping ? 0 : -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -errno;
        }
        if (n == 0 && stopping) return 0;
        for (int i = 0; i < n; i++) {
            switch (evs[i].data.u64) {
            case TAG_SIGNAL:
                if (on_signal(p)) stopping = true;
                break;
            case TAG_FIFO: {
                int err = on_fifo(p);
                if (err) return err;
                break;
            }
            case TAG_LISTEN:
                on_client(p);
                break;
            }
        }
    }
}

void plumb_close(struct plumb *p) {
    if (p->listener >= 0) close(p->listener);
    if (p->sock_path[0]) unlink(p->sock_path);
    if (p->fifo >= 0) close(p->fifo);
    if (p->sig >= 0) close(p->sig);
    if (p->ep >= 0) close(p->ep);
    *p = (struct plumb){ .ep = -1, .sig = -1, .fifo = -1, .listener = -1 };
}

int plumb_main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <fifo> <socket>\n", argc > 0 ? argv[0] : "plumbd");
        return 2;
    }
    struct plumb p;
    int err = plumb_open(&p, argv[1], argv[2]);
    if (err) {
        fprintf(stderr, "%s: cannot start: %s\n", argv[0], strerror(-err));
        return 1;
    }
    err = plumb_run(&p);
    plumb_close(&p);
    return err ? 1 : 0;
}
