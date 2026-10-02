#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#define EVLOOP_MAX 16

/* Called when fd is readable (or hung up / in error). Return false to stop watching it. */
typedef bool (*ev_handler)(int fd, void *ctx);

struct ev_slot {
    int fd;
    ev_handler fn;
    void *ctx;
    bool used;
};

struct evloop {
    int epfd;     /* the epoll instance */
    int stopfd;   /* eventfd: any write means "shut down" */
    struct ev_slot slots[EVLOOP_MAX];
};

struct evloop *evloop_create(void) {
    struct evloop *loop = calloc(1, sizeof *loop);
    if (!loop) return NULL;
    loop->epfd = epoll_create1(EPOLL_CLOEXEC);
    loop->stopfd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    struct epoll_event ev = { .events = EPOLLIN, .data.ptr = NULL };   /* NULL marks the stop fd */
    if (loop->epfd < 0 || loop->stopfd < 0 || epoll_ctl(loop->epfd, EPOLL_CTL_ADD, loop->stopfd, &ev) < 0) {
        if (loop->epfd >= 0) close(loop->epfd);
        if (loop->stopfd >= 0) close(loop->stopfd);
        free(loop);
        return NULL;
    }
    return loop;
}

int evloop_add(struct evloop *loop, int fd, ev_handler fn, void *ctx) {
    for (int i = 0; i < EVLOOP_MAX; i++) {
        struct ev_slot *s = &loop->slots[i];
        if (s->used) continue;
        struct epoll_event ev = { .events = EPOLLIN, .data.ptr = s };   /* level-triggered */
        if (epoll_ctl(loop->epfd, EPOLL_CTL_ADD, fd, &ev) < 0) return -errno;
        *s = (struct ev_slot){ .fd = fd, .fn = fn, .ctx = ctx, .used = true };
        return 0;
    }
    return -ENOSPC;
}

int evloop_request_stop(struct evloop *loop) {
    uint64_t one = 1;
    return write(loop->stopfd, &one, sizeof one) == (ssize_t)sizeof one ? 0 : -errno;
}

int evloop_run(struct evloop *loop) {
    struct epoll_event events[EVLOOP_MAX + 1];
    bool stopping = false;
    int calls = 0;
    for (;;) {
        /* Normally sleep until something happens. Once stopping, only poll: drain what's left. */
        int n = epoll_wait(loop->epfd, events, EVLOOP_MAX + 1, stopping ? 0 : -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -errno;
        }
        if (n == 0 && stopping) return calls;
        for (int i = 0; i < n; i++) {
            struct ev_slot *s = events[i].data.ptr;
            if (s == NULL) {                       /* the shutdown eventfd */
                uint64_t count;
                if (read(loop->stopfd, &count, sizeof count) < 0) { /* already drained */ }
                stopping = true;
                continue;
            }
            if (!s->used) continue;                /* removed earlier in this batch */
            calls++;
            if (!s->fn(s->fd, s->ctx)) {
                epoll_ctl(loop->epfd, EPOLL_CTL_DEL, s->fd, NULL);
                s->used = false;
            }
        }
    }
}

void evloop_destroy(struct evloop *loop) {
    if (!loop) return;
    close(loop->stopfd);
    close(loop->epfd);   /* the watched fds belong to the caller: don't close them */
    free(loop);
}
