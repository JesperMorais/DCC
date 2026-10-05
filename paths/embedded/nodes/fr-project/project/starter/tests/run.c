/* run.c: runs the plant as a child process and parses its output. */
#define _GNU_SOURCE
#include "run.h"

#include <ctype.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_RUNS 8

static run_t *runs[MAX_RUNS];
static int n_runs;

static long now_ms(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

/* strncpy that always terminates, without format-truncation warnings. */
static void copy_str(char *dst, size_t size, const char *src)
{
    size_t len = strlen(src);
    if (len >= size) len = size - 1;
    memcpy(dst, src, len);
    dst[len] = '\0';
}

static void parse_line(run_t *r, const char *s)
{
    char *end;
    long tick = strtol(s, &end, 10);
    bool ok = end != s && *end == ' ' && isupper((unsigned char)end[1]);
    if (!ok) {
        if (r->well_formed) copy_str(r->bad_line, sizeof r->bad_line, s);
        r->well_formed = false;
        return;
    }
    if (r->n >= MAX_LINES) return;
    line_t *l = &r->lines[r->n++];
    l->tick = tick;
    copy_str(l->text, sizeof l->text, end + 1);
    int i = 0;
    while (l->text[i] && l->text[i] != ' ' && i < (int)sizeof l->tag - 1) {
        l->tag[i] = l->text[i];
        i++;
    }
    l->tag[i] = '\0';
}

static void append(char *buf, size_t size, size_t *len, const char *data, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (*len + 1 >= size) {
            memmove(buf, buf + size / 2, *len - size / 2); /* keep the tail */
            *len -= size / 2;
        }
        buf[(*len)++] = data[i];
    }
    buf[*len] = '\0';
}

static void execute(run_t *r)
{
    char *argv[32];
    char copy[128];
    int argc = 0;
    copy_str(copy, sizeof copy, r->args);
    argv[argc++] = "./build/plant";
    for (char *tok = strtok(copy, " "); tok && argc < 31; tok = strtok(NULL, " ")) argv[argc++] = tok;
    argv[argc] = NULL;

    long run_ms = 3000;
    for (int i = 1; i + 1 < argc; i++)
        if (strcmp(argv[i], "--run-ms") == 0) run_ms = atol(argv[i + 1]);
    long deadline = now_ms() + 2 * run_ms + 4000;

    int out[2], err[2];
    if (pipe(out) || pipe(err)) {
        perror("pipe");
        exit(2);
    }
    pid_t pid = fork();
    if (pid == 0) {
        dup2(out[1], 1);
        dup2(err[1], 2);
        close(out[0]);
        close(err[0]);
        close(out[1]);
        close(err[1]);
        execv(argv[0], argv);
        fprintf(stderr, "cannot run %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }
    close(out[1]);
    close(err[1]);

    static char stdout_buf[1 << 18], stderr_buf[1 << 14];
    size_t out_len = 0, err_len = 0;
    stdout_buf[0] = stderr_buf[0] = '\0';
    struct pollfd fds[2] = { { out[0], POLLIN, 0 }, { err[0], POLLIN, 0 } };
    int open_fds = 2;
    bool killed = false;
    while (open_fds > 0) {
        long left = deadline - now_ms();
        if (left <= 0) {
            kill(pid, SIGKILL);
            killed = true;
            break;
        }
        if (poll(fds, 2, (int)left) < 0 && errno != EINTR) break;
        for (int i = 0; i < 2; i++) {
            if (fds[i].fd < 0 || !(fds[i].revents & (POLLIN | POLLHUP))) continue;
            char chunk[4096];
            ssize_t got = read(fds[i].fd, chunk, sizeof chunk);
            if (got <= 0) {
                close(fds[i].fd);
                fds[i].fd = -1;
                open_fds--;
            } else if (i == 0) {
                if (out_len + (size_t)got < sizeof stdout_buf) {
                    memcpy(stdout_buf + out_len, chunk, (size_t)got);
                    out_len += (size_t)got;
                    stdout_buf[out_len] = '\0';
                }
            } else {
                append(stderr_buf, sizeof stderr_buf, &err_len, chunk, (size_t)got);
            }
        }
    }
    for (int i = 0; i < 2; i++)
        if (fds[i].fd >= 0) close(fds[i].fd);

    int status = 0;
    waitpid(pid, &status, 0);
    r->finished = !killed && WIFEXITED(status);
    r->exit_code = r->finished ? WEXITSTATUS(status) : -1;

    r->well_formed = true;
    for (char *line = strtok(stdout_buf, "\n"); line; line = strtok(NULL, "\n")) parse_line(r, line);

    const char *hw_line = strstr(stderr_buf, "hw: ticks=");
    if (hw_line) {
        size_t len = strcspn(hw_line, "\n");
        if (len >= sizeof r->hw) len = sizeof r->hw - 1;
        memcpy(r->hw, hw_line, len);
        r->hw[len] = '\0';
    }
    const char *tail = err_len > 300 ? stderr_buf + err_len - 300 : stderr_buf;
    copy_str(r->err_tail, sizeof r->err_tail, tail);
    for (char *c = r->err_tail; *c; c++)
        if (*c == '\n') *c = ' ';
    if (killed) snprintf(r->err_tail, sizeof r->err_tail, "still running after %ld ms, killed", 2 * run_ms + 4000);
}

const run_t *plant(const char *args)
{
    for (int i = 0; i < n_runs; i++)
        if (strcmp(runs[i]->args, args) == 0) return runs[i];
    run_t *r = calloc(1, sizeof *r);
    copy_str(r->args, sizeof r->args, args);
    execute(r);
    if (n_runs < MAX_RUNS) runs[n_runs++] = r;
    return r;
}

int find(const run_t *r, const char *tag, const line_t **out, int max)
{
    int k = 0;
    for (int i = 0; i < r->n && k < max; i++)
        if (strcmp(r->lines[i].tag, tag) == 0) out[k++] = &r->lines[i];
    return k;
}

int count(const run_t *r, const char *tag)
{
    int k = 0;
    for (int i = 0; i < r->n; i++) k += strcmp(r->lines[i].tag, tag) == 0;
    return k;
}

const line_t *last_line(const run_t *r) { return r->n ? &r->lines[r->n - 1] : NULL; }

static const char *value_of(const char *text, const char *key)
{
    size_t klen = strlen(key);
    for (const char *p = text; (p = strstr(p, key)) != NULL; p++)
        if ((p == text || p[-1] == ' ') && p[klen] == '=') return p + klen + 1;
    return NULL;
}

long kv(const char *text, const char *key, long dflt)
{
    const char *v = value_of(text, key);
    char *end;
    if (!v) return dflt;
    long x = strtol(v, &end, 10);
    return end == v ? dflt : x;
}

const char *nth_word(const char *text, int n, char *buf, int size)
{
    const char *p = text;
    for (int i = 0; i < n && p; i++) {
        p = strchr(p, ' ');
        if (p) p++;
    }
    int len = 0;
    while (p && p[len] && p[len] != ' ' && len < size - 1) {
        buf[len] = p[len];
        len++;
    }
    buf[len] = '\0';
    return buf;
}

long hw(const run_t *r, const char *key, long dflt) { return kv(r->hw, key, dflt); }

const char *describe(const run_t *r)
{
    static char buf[800];
    if (!r->finished) snprintf(buf, sizeof buf, "./build/plant %s: %s", r->args, r->err_tail);
    else snprintf(buf, sizeof buf, "./build/plant %s exited with %d, %d lines; stderr: %s", r->args,
                  r->exit_code, r->n, r->err_tail[0] ? r->err_tail : "(empty)");
    return buf;
}
