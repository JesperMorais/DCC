#include "harness.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int check_pass_, check_fail_, check_num_;
bool check_ok_;

static char sensord_path[PATH_MAX];
static char home_dir[PATH_MAX];
static char test_dir[PATH_MAX];
static pid_t daemon_pid = -1;
static int daemon_status;
static bool daemon_reaped;

void harness_init(void) {
    if (!realpath("sensord", sensord_path)) {
        printf("Bail out! ./sensord not found: run the tests through make\n");
        exit(1);
    }
    if (!getcwd(home_dir, sizeof home_dir)) exit(1);
    signal(SIGPIPE, SIG_IGN); /* a dead daemon must fail a test, not kill the runner */
}

void sleep_ms(int ms) {
    struct timespec ts = { ms / 1000, (long)(ms % 1000) * 1000000L };
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {}
}

long now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static int rm_entry(const char *path, const struct stat *st, int flag, struct FTW *ftw) {
    (void)st; (void)flag; (void)ftw;
    return remove(path);
}

void test_begin(void) {
    snprintf(test_dir, sizeof test_dir, "/tmp/sensord-test-XXXXXX");
    if (!mkdtemp(test_dir) || chdir(test_dir) != 0) {
        printf("Bail out! cannot create a temp dir\n");
        exit(1);
    }
    daemon_pid = -1;
    daemon_reaped = false;
}

void test_end(void) {
    if (daemon_pid > 0 && !daemon_reaped) kill_daemon();
    if (!check_ok_) {
        char *err = read_file("err.txt");
        if (err && *err) {
            printf("#   sensord's stderr:\n");
            for (char *line = strtok(err, "\n"); line; line = strtok(NULL, "\n")) printf("#     %s\n", line);
        }
        free(err);
    }
    if (chdir(home_dir) != 0) exit(1);
    nftw(test_dir, rm_entry, 16, FTW_DEPTH | FTW_PHYS);
}

void write_file(const char *path, const char *fmt, ...) {
    FILE *f = fopen(path, "w");
    if (!f) {
        printf("Bail out! cannot write %s: %s\n", path, strerror(errno));
        exit(1);
    }
    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fclose(f);
}

char *read_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    size_t cap = 4096, len = 0;
    char *buf = malloc(cap);
    size_t n;
    while ((n = fread(buf + len, 1, cap - len - 1, f)) > 0) {
        len += n;
        if (cap - len < 2) buf = realloc(buf, cap *= 2);
    }
    buf[len] = '\0';
    fclose(f);
    return buf;
}

bool exists(const char *path) {
    struct stat st;
    return lstat(path, &st) == 0;
}

int count_lines(const char *text) {
    if (!text) return 0;
    int n = 0;
    for (const char *p = text; *p; p++)
        if (*p == '\n') n++;
    if (*text && text[strlen(text) - 1] != '\n') n++;
    return n;
}

void fake_temp(const char *dir, const char *raw, const char *scale, const char *offset) {
    char path[PATH_MAX];
    mkdir(dir, 0755);
    snprintf(path, sizeof path, "%s/name", dir);
    write_file(path, "fake-temp\n");
    snprintf(path, sizeof path, "%s/in_temp_raw", dir);
    if (raw) write_file(path, "%s\n", raw); else unlink(path);
    snprintf(path, sizeof path, "%s/in_temp_scale", dir);
    if (scale) write_file(path, "%s\n", scale); else unlink(path);
    snprintf(path, sizeof path, "%s/in_temp_offset", dir);
    if (offset) write_file(path, "%s\n", offset); else unlink(path);
}

void write_conf(int period_ms, const char *extra) {
    write_file("sensord.conf",
               "# written by the tests\n"
               "fifo = sensor.fifo\n"
               "sysfs = iio\n"
               "log_dir = logs\n"
               "period_ms = %d\n"
               "%s",
               period_ms, extra ? extra : "");
}

static pid_t spawn(const char *arg1, const char *arg2) {
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        int out = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        int err = open("err.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        int null = open("/dev/null", O_RDONLY);
        dup2(null, 0);
        dup2(out, 1);
        dup2(err, 2);
        for (int fd = 3; fd < 256; fd++) close(fd);
        signal(SIGPIPE, SIG_DFL);
        sigset_t none;
        sigemptyset(&none);
        sigprocmask(SIG_SETMASK, &none, NULL);
        execl(sensord_path, "sensord", arg1, arg2, (char *)NULL);
        _exit(127);
    }
    return pid;
}

static int wait_child(pid_t pid, int timeout_ms, int *status) {
    long until = now_ms() + timeout_ms;
    for (;;) {
        pid_t r = waitpid(pid, status, WNOHANG);
        if (r == pid) return 0;
        if (now_ms() >= until) return -1;
        sleep_ms(5);
    }
}

static int exit_code(int status) {
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

int run_once(const char *arg1, const char *arg2, int timeout_ms) {
    pid_t pid = spawn(arg1, arg2);
    int status;
    if (wait_child(pid, timeout_ms, &status) != 0) {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        printf("#   sensord didn't exit within %d ms\n", timeout_ms);
        return -1;
    }
    return exit_code(status);
}

pid_t start_daemon(void) {
    daemon_pid = spawn("sensord.conf", NULL);
    daemon_reaped = false;
    return daemon_pid;
}

pid_t daemon_pid_for_tests(void) {
    return daemon_pid;
}

bool daemon_alive(void) {
    if (daemon_pid <= 0 || daemon_reaped) return false;
    if (waitpid(daemon_pid, &daemon_status, WNOHANG) == daemon_pid) {
        daemon_reaped = true;
        return false;
    }
    return true;
}

int fifo_open(int timeout_ms) {
    long until = now_ms() + timeout_ms;
    for (;;) {
        int fd = open("sensor.fifo", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd >= 0) return fd;
        if (now_ms() >= until || (daemon_pid > 0 && !daemon_alive())) return -1;
        sleep_ms(10);
    }
}

void fifo_send(int fd, const char *text) {
    size_t len = strlen(text), off = 0;
    long until = now_ms() + 1000;
    while (off < len && now_ms() < until) {
        ssize_t n = write(fd, text + off, len - off);
        if (n > 0) off += (size_t)n;
        else if (n < 0 && errno == EPIPE) {
            CHECKM(false, "writing to sensor.fifo failed with EPIPE: for a moment nobody had it open for reading");
            return;
        } else if (n < 0 && errno != EAGAIN && errno != EINTR) break;
        else sleep_ms(5);
    }
}

bool wait_ready(int timeout_ms) {
    int fd = fifo_open(timeout_ms);
    if (fd < 0) {
        printf("#   sensord never opened sensor.fifo for reading%s\n", daemon_alive() ? "" : " (it exited)");
        return false;
    }
    close(fd);
    if (!WAIT_UNTIL(exists("logs/sensord.csv"), timeout_ms)) {
        printf("#   logs/sensord.csv never appeared\n");
        return false;
    }
    return true;
}

int stop_daemon(int sig, int timeout_ms) {
    if (!daemon_alive()) {
        printf("#   sensord had already exited\n");
        return exit_code(daemon_status);
    }
    kill(daemon_pid, sig);
    if (wait_child(daemon_pid, timeout_ms, &daemon_status) != 0) {
        printf("#   sensord didn't exit within %d ms of signal %d\n", timeout_ms, sig);
        kill_daemon();
        return -1;
    }
    daemon_reaped = true;
    return exit_code(daemon_status);
}

int daemon_exit_code(void) {
    if (daemon_alive() || !daemon_reaped) return -1;
    return exit_code(daemon_status);
}

void kill_daemon(void) {
    if (daemon_pid <= 0 || daemon_reaped) return;
    kill(daemon_pid, SIGKILL);
    waitpid(daemon_pid, &daemon_status, 0);
    daemon_reaped = true;
}

static const char *HEADER = "tick,temp,count,min,mean,max";

/* Copies the next comma-separated field into out and advances *p. */
static bool field(char **p, char *out, size_t len, bool last) {
    char *end = last ? *p + strlen(*p) : strchr(*p, ',');
    if (!end) return false;
    size_t n = (size_t)(end - *p);
    if (n >= len) return false;
    memcpy(out, *p, n);
    out[n] = '\0';
    *p = *end ? end + 1 : end;
    return true;
}

int load_csv(const char *path, struct row *rows, int max) {
    char *text = read_file(path);
    if (!text) return -1;
    char *save, *line = strtok_r(text, "\n", &save);
    if (!line || strcmp(line, HEADER) != 0) {
        free(text);
        return -1;
    }
    int n = 0;
    while ((line = strtok_r(NULL, "\n", &save)) && n < max) {
        struct row r;
        char tick[32], count[32], *p = line, *end;
        if (!field(&p, tick, sizeof tick, false) || !field(&p, r.temp, sizeof r.temp, false) ||
            !field(&p, count, sizeof count, false) || !field(&p, r.min, sizeof r.min, false) ||
            !field(&p, r.mean, sizeof r.mean, false) || !field(&p, r.max, sizeof r.max, true)) {
            printf("#   %s: malformed row \"%s\"\n", path, line);
            free(text);
            return -2;
        }
        r.tick = strtol(tick, &end, 10);
        if (*tick == '\0' || *end) { free(text); printf("#   %s: bad tick in \"%s\"\n", path, line); return -2; }
        r.count = (int)strtol(count, &end, 10);
        if (*count == '\0' || *end) { free(text); printf("#   %s: bad count in \"%s\"\n", path, line); return -2; }
        rows[n++] = r;
    }
    free(text);
    return n;
}

int csv_rows(const char *path) {
    static struct row rows[MAX_ROWS];
    return load_csv(path, rows, MAX_ROWS);
}

int total_count(const struct row *rows, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) total += rows[i].count;
    return total;
}

long daemon_cpu_ms(void) {
    char path[64];
    snprintf(path, sizeof path, "/proc/%d/stat", (int)daemon_pid);
    char *text = read_file(path);
    if (!text) return -1;
    /* Fields after the ")" of the command name: state is field 3, utime 14, stime 15. */
    char *p = strrchr(text, ')');
    unsigned long utime = 0, stime = 0;
    if (p) sscanf(p + 2, "%*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", &utime, &stime);
    free(text);
    return (long)((utime + stime) * 1000 / (unsigned long)sysconf(_SC_CLK_TCK));
}

int daemon_fd_count(void) {
    char path[64];
    snprintf(path, sizeof path, "/proc/%d/fd", (int)daemon_pid);
    DIR *d = opendir(path);
    if (!d) return -1;
    int n = 0;
    struct dirent *e;
    while ((e = readdir(d)))
        if (e->d_name[0] != '.') n++;
    closedir(d);
    return n;
}

int sock_connect(const char *path, int timeout_ms) {
    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    snprintf(addr.sun_path, sizeof addr.sun_path, "%s", path);
    long until = now_ms() + timeout_ms;
    for (;;) {
        int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if (connect(fd, (struct sockaddr *)&addr, sizeof addr) == 0) return fd;
        close(fd);
        if (now_ms() >= until || !daemon_alive()) return -1;
        sleep_ms(10);
    }
}

bool sock_send(int fd, const char *text) {
    size_t len = strlen(text);
    return write(fd, text, len) == (ssize_t)len;
}

bool sock_line(int fd, char *buf, int len, int timeout_ms) {
    int n = 0;
    long until = now_ms() + timeout_ms;
    while (n < len - 1) {
        int left = (int)(until - now_ms());
        struct pollfd p = { .fd = fd, .events = POLLIN };
        if (left <= 0 || poll(&p, 1, left) <= 0) break;
        char c;
        if (read(fd, &c, 1) != 1) break;
        if (c == '\n') {
            buf[n] = '\0';
            return true;
        }
        buf[n++] = c;
    }
    buf[n] = '\0';
    return false;
}
