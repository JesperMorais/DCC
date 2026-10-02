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

/* A suggested layout. Change it however you like: the tests only use the functions. */
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
    /* TODO: epoll_create1, eventfd, and register the eventfd with epoll */
    return calloc(1, sizeof(struct evloop));
}

int evloop_add(struct evloop *loop, int fd, ev_handler fn, void *ctx) {
    (void)loop;
    (void)fd;
    (void)fn;
    (void)ctx;
    return -ENOSYS;
}

int evloop_request_stop(struct evloop *loop) {
    (void)loop;
    return -ENOSYS;
}

int evloop_run(struct evloop *loop) {
    (void)loop;
    return -ENOSYS;
}

void evloop_destroy(struct evloop *loop) {
    free(loop);
}
