#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

/* Each test runs in its own process. They all share one temp directory, so
 * every test uses its own file names and removes leftovers first. */
static void fresh(const char *fifo, const char *sock) {
    unlink(fifo);
    unlink(sock);
}

static int open_fd_count(void) {
    int n = 0;
    DIR *d = opendir("/proc/self/fd");
    if (!d) return -1;
    while (readdir(d)) n++;
    closedir(d);
    return n;
}

/* A writer the way a sensor would connect: never waits for a reader. -1 + ENXIO if there's none. */
static int writer(const char *fifo) {
    return open(fifo, O_WRONLY | O_NONBLOCK | O_CLOEXEC);
}

static void put(int fd, const char *text) {
    if (write(fd, text, strlen(text)) < 0) perror("write");
}

static int client(const char *sock) {
    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    snprintf(addr.sun_path, sizeof addr.sun_path, "%s", sock);
    int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (connect(fd, (struct sockaddr *)&addr, sizeof addr) != 0) {
        close(fd);
        return -1;
    }
    return fd;
}

/* Reads everything until the server closes the connection (or 500 ms pass). */
static void read_reply(int fd, char *buf, size_t len) {
    size_t got = 0;
    struct pollfd pfd = { .fd = fd, .events = POLLIN };
    while (got < len - 1 && poll(&pfd, 1, 500) == 1) {
        ssize_t n = read(fd, buf + got, len - 1 - got);
        if (n <= 0) break;
        got += (size_t)n;
    }
    buf[got] = '\0';
}

static bool is_blocked(int sig) {
    sigset_t now;
    sigprocmask(SIG_BLOCK, NULL, &now);
    return sigismember(&now, sig) == 1;
}

TEST(open_creates_the_fifo_and_the_socket) {
    fresh("t1.fifo", "t1.sock");
    struct plumb p;
    EXPECT_EQ(plumb_open(&p, "t1.fifo", "t1.sock"), 0);
    struct stat st;
    EXPECT_EQ(lstat("t1.fifo", &st), 0);
    EXPECT_TRUE(S_ISFIFO(st.st_mode));
    EXPECT_EQ(lstat("t1.sock", &st), 0);
    EXPECT_TRUE(S_ISSOCK(st.st_mode));
    int w = writer("t1.fifo");
    EXPECT_TRUE(w >= 0);                     /* someone is reading, so a writer may connect */
    close(w);
    plumb_close(&p);
    EXPECT_EQ(lstat("t1.sock", &st), -1);    /* the socket file goes with the daemon */
    EXPECT_EQ(lstat("t1.fifo", &st), 0);     /* the FIFO stays: writers may be waiting for it */
}

TEST(signals_are_blocked_and_read_as_data) {
    fresh("t2.fifo", "t2.sock");
    struct plumb p;
    EXPECT_EQ(plumb_open(&p, "t2.fifo", "t2.sock"), 0);
    EXPECT_TRUE(is_blocked(SIGTERM));
    EXPECT_TRUE(is_blocked(SIGINT));
    EXPECT_TRUE(is_blocked(SIGHUP));
    raise(SIGHUP);                           /* blocked: it waits in the signalfd */
    raise(SIGTERM);
    EXPECT_EQ(plumb_run(&p), 0);
    EXPECT_EQ(p.hups, 1);
    raise(SIGHUP);
    raise(SIGHUP);                           /* a standard signal is pending or not: these two are one */
    raise(SIGINT);                           /* Ctrl-C stops it too */
    EXPECT_EQ(plumb_run(&p), 0);
    EXPECT_EQ(p.hups, 2);
    plumb_close(&p);
}

TEST(counts_lines_and_drains_them_before_stopping) {
    fresh("t3.fifo", "t3.sock");
    struct plumb p;
    EXPECT_EQ(plumb_open(&p, "t3.fifo", "t3.sock"), 0);
    int w = writer("t3.fifo");
    EXPECT_TRUE(w >= 0);
    put(w, "21.5\n22.0\n22.");
    put(w, "5\n");
    raise(SIGTERM);                          /* already waiting: the lines must still be counted */
    EXPECT_EQ(plumb_run(&p), 0);
    EXPECT_EQ(p.lines, 3);
    close(w);
    plumb_close(&p);
}

TEST(a_writer_leaving_is_not_the_end) {
    fresh("t4.fifo", "t4.sock");
    struct plumb p;
    EXPECT_EQ(plumb_open(&p, "t4.fifo", "t4.sock"), 0);
    int fds = open_fd_count();
    for (int i = 0; i < 20; i++) {
        int w = writer("t4.fifo");
        EXPECT_TRUE(w >= 0);                 /* -1 (ENXIO): nobody had the FIFO open for reading */
        put(w, "1\n");
        close(w);                            /* EOF: the reader sees read() == 0, forever */
        raise(SIGTERM);
        EXPECT_EQ(plumb_run(&p), 0);         /* a timeout here: EOF kept the loop spinning */
        EXPECT_EQ(p.lines, i + 1);
    }
    EXPECT_EQ(open_fd_count(), fds);         /* one read end at a time, old ones closed */
    plumb_close(&p);
}

TEST(the_new_read_end_opens_before_the_old_one_closes) {
    fresh("t5.fifo", "t5.sock");
    struct plumb p;
    EXPECT_EQ(plumb_open(&p, "t5.fifo", "t5.sock"), 0);
    int in = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    EXPECT_TRUE(in >= 0);
    EXPECT_TRUE(inotify_add_watch(in, "t5.fifo", IN_OPEN | IN_CLOSE_NOWRITE) >= 0);
    int w = writer("t5.fifo");
    EXPECT_TRUE(w >= 0);
    put(w, "7\n");
    close(w);
    char skip[4096];
    while (read(in, skip, sizeof skip) > 0) {}   /* forget our own writer's open */
    raise(SIGTERM);
    EXPECT_EQ(plumb_run(&p), 0);
    /* Watch what happened to the FIFO while plumb_run handled the EOF. */
    char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
    ssize_t n = read(in, buf, sizeof buf);
    bool opened = false, closed_while_alone = false;
    for (char *e = buf; n > 0 && e < buf + n; e += sizeof(struct inotify_event) + ((struct inotify_event *)e)->len) {
        uint32_t mask = ((struct inotify_event *)e)->mask;
        if (mask & IN_OPEN) opened = true;
        if ((mask & IN_CLOSE_NOWRITE) && !opened) closed_while_alone = true;
    }
    close(in);
    /* Closing the old read end first leaves a moment with no reader at all. */
    EXPECT_FALSE(closed_while_alone);
    EXPECT_EQ(p.lines, 1);
    plumb_close(&p);
}

TEST(answers_a_client_on_the_socket) {
    fresh("t6.fifo", "t6.sock");
    struct plumb p;
    EXPECT_EQ(plumb_open(&p, "t6.fifo", "t6.sock"), 0);
    int w = writer("t6.fifo");
    put(w, "1\n2\n");
    raise(SIGHUP);
    raise(SIGTERM);
    EXPECT_EQ(plumb_run(&p), 0);
    int c1 = client("t6.sock");
    int c2 = client("t6.sock");              /* the kernel queues both until accept() */
    EXPECT_TRUE(c1 >= 0 && c2 >= 0);
    raise(SIGTERM);
    EXPECT_EQ(plumb_run(&p), 0);
    char reply[128];
    read_reply(c1, reply, sizeof reply);
    EXPECT_STR_EQ(reply, "lines=2 hups=1\n");    /* then the server hangs up */
    read_reply(c2, reply, sizeof reply);
    EXPECT_STR_EQ(reply, "lines=2 hups=1\n");
    close(c1);
    close(c2);
    close(w);
    plumb_close(&p);
}

TEST(replaces_a_stale_socket_but_never_a_file) {
    fresh("t7.fifo", "t7.sock");
    /* A crashed daemon leaves its socket file behind. */
    int old = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = { .sun_family = AF_UNIX, .sun_path = "t7.sock" };
    EXPECT_EQ(bind(old, (struct sockaddr *)&addr, sizeof addr), 0);
    close(old);
    struct plumb p;
    EXPECT_EQ(plumb_open(&p, "t7.fifo", "t7.sock"), 0);
    int c = client("t7.sock");
    EXPECT_TRUE(c >= 0);
    close(c);
    plumb_close(&p);

    /* Someone's config file at the socket path is not ours to delete. */
    int fds = open_fd_count();
    int f = open("t7.sock", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    put(f, "precious\n");
    close(f);
    EXPECT_EQ(plumb_open(&p, "t7.fifo", "t7.sock"), -EEXIST);
    struct stat st;
    EXPECT_EQ(lstat("t7.sock", &st), 0);
    EXPECT_TRUE(S_ISREG(st.st_mode) && st.st_size == 9);
    /* Nor is a regular file where the FIFO should be. */
    unlink("t7.sock");
    unlink("t7.fifo");
    f = open("t7.fifo", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    close(f);
    EXPECT_EQ(plumb_open(&p, "t7.fifo", "t7.sock"), -EEXIST);
    EXPECT_EQ(lstat("t7.sock", &st), -1);    /* it gave up before creating the socket */
    EXPECT_EQ(open_fd_count(), fds);         /* and closed what it had opened */
}

TEST(main_turns_the_outcome_into_an_exit_code) {
    fresh("t8.fifo", "t8.sock");
    char *few[] = { "plumbd", "t8.fifo", NULL };
    EXPECT_EQ(plumb_main(2, few), 2);        /* usage error */
    int f = open("t8.sock", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    close(f);
    char *args[] = { "plumbd", "t8.fifo", "t8.sock", NULL };
    EXPECT_EQ(plumb_main(3, args), 1);       /* can't start */
    unlink("t8.sock");
    /* Queue a SIGTERM first (blocked, so it waits): main sets up, sees it, shuts down cleanly. */
    sigset_t term;
    sigemptyset(&term);
    sigaddset(&term, SIGTERM);
    sigprocmask(SIG_BLOCK, &term, NULL);
    raise(SIGTERM);
    EXPECT_EQ(plumb_main(3, args), 0);
    struct stat st;
    EXPECT_EQ(lstat("t8.sock", &st), -1);    /* clean shutdown removed the socket */
}
