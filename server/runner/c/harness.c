/* daily.ts C harness runtime: runs each registered TEST in its own forked process so a crash,
 * sanitizer error or infinite loop only fails that one test. Results go to fd 3, one line per
 * test, fields separated by \x1f: name, status, ms, message, expected, received, detail. */
#define _GNU_SOURCE
#include "harness.h"

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#if defined(__has_include)
#if __has_include(<sanitizer/lsan_interface.h>)
#include <sanitizer/lsan_interface.h>
#define DTS_HAVE_LSAN 1
#endif
#endif

#define DTS_MAX_TESTS 256
#define DTS_TIMEOUT_MS 1500

static struct { const char *name; dts_test_fn fn; int line; } dts_tests[DTS_MAX_TESTS];
static int dts_count;
static int dts_result_fd = -1; /* child → parent pipe for the current test */

void dts_register(const char *name, dts_test_fn fn, int line) {
    if (dts_count < DTS_MAX_TESTS) {
        dts_tests[dts_count].name = name;
        dts_tests[dts_count].fn = fn;
        dts_tests[dts_count].line = line;
        dts_count++;
    }
}

bool dts_str_eq(const char *a, const char *b) {
    if (a == NULL || b == NULL) return a == b;
    return strcmp(a, b) == 0;
}

bool dts_arr_eq(const int *a, const int *b, size_t n) {
    if (n == 0) return true;
    if (a == NULL || b == NULL) return a == b;
    for (size_t i = 0; i < n; i++)
        if (a[i] != b[i]) return false;
    return true;
}

/* Child side: report a failure (status F) and stop. */
static void dts_report(const char *status, const char *message, const char *expected, const char *received) {
    char buf[8192];
    int len = snprintf(buf, sizeof buf, "%s\x1f%s\x1f%s\x1f%s", status, message ? message : "", expected ? expected : "", received ? received : "");
    if (len > (int)sizeof buf - 1) len = (int)sizeof buf - 1;
    if (dts_result_fd >= 0 && write(dts_result_fd, buf, (size_t)len) < 0) { /* nothing sensible to do */ }
}

static void dts_failf(int line, const char *expr, const char *expected, const char *received) {
    char msg[1024];
    snprintf(msg, sizeof msg, "tests.c line %d: EXPECT failed: %s", line, expr);
    dts_report("F", msg, expected, received);
    fflush(stdout);
    _exit(0);
}

void dts_fail(int line, const char *expr, const char *expected, const char *received) { dts_failf(line, expr, expected, received); }

void dts_fail_ll(int line, const char *expr, long long expected, long long received) {
    char e[64], r[64];
    snprintf(e, sizeof e, "%lld", expected);
    snprintf(r, sizeof r, "%lld", received);
    dts_failf(line, expr, e, r);
}

void dts_fail_ull(int line, const char *expr, unsigned long long expected, unsigned long long received) {
    char e[64], r[64];
    snprintf(e, sizeof e, "%llu (0x%llx)", expected, expected);
    snprintf(r, sizeof r, "%llu (0x%llx)", received, received);
    dts_failf(line, expr, e, r);
}

void dts_fail_dbl(int line, const char *expr, double expected, double received) {
    char e[64], r[64];
    snprintf(e, sizeof e, "%.10g", expected);
    snprintf(r, sizeof r, "%.10g", received);
    dts_failf(line, expr, e, r);
}

static void dts_quote(char *out, size_t cap, const char *s) {
    if (s == NULL) {
        snprintf(out, cap, "NULL");
        return;
    }
    size_t j = 0;
    out[j++] = '"';
    for (size_t i = 0; s[i] && j + 6 < cap; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == '\n') { out[j++] = '\\'; out[j++] = 'n'; }
        else if (c == '\t') { out[j++] = '\\'; out[j++] = 't'; }
        else if (c == '"' || c == '\\') { out[j++] = '\\'; out[j++] = (char)c; }
        else if (c < 32) j += (size_t)snprintf(out + j, cap - j, "\\x%02x", c);
        else out[j++] = (char)c;
    }
    out[j++] = '"';
    out[j] = '\0';
}

void dts_fail_str(int line, const char *expr, const char *expected, const char *received) {
    char e[2048], r[2048];
    dts_quote(e, sizeof e, expected);
    dts_quote(r, sizeof r, received);
    dts_failf(line, expr, e, r);
}

void dts_fail_ptr(int line, const char *expr, const void *expected, const void *received) {
    char e[64], r[64];
    if (expected) snprintf(e, sizeof e, "%p", expected); else snprintf(e, sizeof e, "NULL");
    if (received) snprintf(r, sizeof r, "%p", received); else snprintf(r, sizeof r, "NULL");
    dts_failf(line, expr, e, r);
}

static void dts_fmt_arr(char *out, size_t cap, const int *a, size_t n) {
    if (a == NULL) {
        snprintf(out, cap, "NULL");
        return;
    }
    size_t j = (size_t)snprintf(out, cap, "{");
    for (size_t i = 0; i < n && j + 16 < cap; i++) j += (size_t)snprintf(out + j, cap - j, i ? ", %d" : "%d", a[i]);
    snprintf(out + j, cap - j, "}");
}

void dts_fail_arr(int line, const char *expr, const int *expected, const int *received, size_t n) {
    char e[2048], r[2048];
    dts_fmt_arr(e, sizeof e, expected, n);
    dts_fmt_arr(r, sizeof r, received, n);
    dts_failf(line, expr, e, r);
}

static int dts_by_line(const void *a, const void *b) {
    const int la = ((const typeof(dts_tests[0]) *)a)->line, lb = ((const typeof(dts_tests[0]) *)b)->line;
    return (la > lb) - (la < lb);
}

static double dts_now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

static size_t dts_read_all(int fd, char *buf, size_t cap) {
    size_t n = 0;
    ssize_t r;
    while ((r = read(fd, buf + n, cap - 1 - n)) > 0) {
        n += (size_t)r;
        if (n >= cap - 1) {
            char sink[4096];
            while (read(fd, sink, sizeof sink) > 0) { /* drain */ }
            break;
        }
    }
    buf[n] = '\0';
    return n;
}

/* Replace field separators / newlines that would break the line protocol. */
static void dts_sanitize(char *s) {
    for (; *s; s++)
        if (*s == '\x1f' || *s == '\x1e') *s = ' ';
}

int main(void) {
    FILE *out = fdopen(3, "w");
    if (!out) out = stderr;
    qsort(dts_tests, (size_t)dts_count, sizeof dts_tests[0], dts_by_line);

    bool looped = false;
    for (int i = 0; i < dts_count; i++) {
        if (looped) {
            fprintf(out, "%s\x1fS\x1f" "0\x1f\x1f\x1f\x1f\x1e", dts_tests[i].name);
            continue;
        }
        int res[2], err[2];
        if (pipe(res) != 0 || pipe(err) != 0) return 2;
        fflush(stdout);
        fflush(out);
        double t0 = dts_now_ms();
        pid_t pid = fork();
        if (pid == 0) {
            close(res[0]);
            close(err[0]);
            dts_result_fd = res[1];
            dup2(err[1], STDERR_FILENO);
            struct itimerval tv = {{0, 0}, {DTS_TIMEOUT_MS / 1000, (DTS_TIMEOUT_MS % 1000) * 1000}};
            setitimer(ITIMER_REAL, &tv, NULL); /* SIGALRM's default action kills us */
            dts_tests[i].fn();
            struct itimerval off = {{0, 0}, {0, 0}};
            setitimer(ITIMER_REAL, &off, NULL);
            fflush(stdout);
#ifdef DTS_HAVE_LSAN
            if (__lsan_do_recoverable_leak_check()) {
                dts_report("L", "", "", "");
                _exit(0);
            }
#endif
            dts_report("P", "", "", "");
            _exit(0);
        }
        close(res[1]);
        close(err[1]);
        static char rbuf[8192], ebuf[16384];
        dts_read_all(res[0], rbuf, sizeof rbuf);
        dts_read_all(err[0], ebuf, sizeof ebuf);
        close(res[0]);
        close(err[0]);
        int status = 0;
        waitpid(pid, &status, 0);
        double ms = dts_now_ms() - t0;
        dts_sanitize(ebuf);

        if (rbuf[0] == 'P' || rbuf[0] == 'F' || rbuf[0] == 'L') {
            /* rbuf is already "status\x1fmessage\x1fexpected\x1freceived" */
            fprintf(out, "%s\x1f%s\x1f%.2f\x1f%s\x1f%s\x1e", dts_tests[i].name, rbuf[0] == 'L' ? "L" : (rbuf[0] == 'P' ? "P" : "F"),
                    ms, rbuf[0] == 'F' ? rbuf + 2 : "\x1f\x1f", rbuf[0] == 'L' ? ebuf : "");
        } else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM) {
            looped = true;
            fprintf(out, "%s\x1fT\x1f%.2f\x1f\x1f\x1f\x1f%s\x1e", dts_tests[i].name, ms, ebuf);
        } else {
            /* Crashed: sanitizer report or a raw signal. */
            fprintf(out, "%s\x1f" "C\x1f%.2f\x1f%s\x1f\x1f\x1f%s\x1e", dts_tests[i].name, ms,
                    WIFSIGNALED(status) ? strsignal(WTERMSIG(status)) : "exited", ebuf);
        }
        fflush(out);
    }
    fflush(out);
    return 0;
}
