#include "csvlog.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util.h"

static const char HEADER[] = "tick,temp,count,min,mean,max\n";

/* Opens log->path and counts the rows already in it. */
static int open_current(struct csvlog *log) {
    log->fd = open(log->path, O_RDWR | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
    if (log->fd < 0) return -1;
    int lines = 0;
    char buf[4096];
    ssize_t n;
    while ((n = read(log->fd, buf, sizeof buf)) > 0)
        for (ssize_t i = 0; i < n; i++) lines += buf[i] == '\n';
    if (lines == 0) {
        log->rows = 0;
        return write_all(log->fd, HEADER, sizeof HEADER - 1);
    }
    log->rows = lines - 1;
    return 0;
}

static void rotated_name(const struct csvlog *log, int i, char *out, size_t len) {
    snprintf(out, len, "%s.%d", log->path, i);
}

static int rotate(struct csvlog *log) {
    char from[PATH_MAX + 16], to[PATH_MAX + 16];
    close(log->fd);
    log->fd = -1;
    rotated_name(log, log->keep, to, sizeof to);
    unlink(to);
    for (int i = log->keep - 1; i >= 1; i--) {
        rotated_name(log, i, from, sizeof from);
        rotated_name(log, i + 1, to, sizeof to);
        rename(from, to); /* ENOENT is fine: not that many files yet */
    }
    rotated_name(log, 1, to, sizeof to);
    if (rename(log->path, to) != 0) return -1;
    log->rotations++;
    return open_current(log);
}

int csvlog_open(struct csvlog *log, const char *dir, int rotate_lines, int keep) {
    *log = (struct csvlog){ .fd = -1, .rotate_lines = rotate_lines, .keep = keep };
    if (mkdir(dir, 0755) != 0 && errno != EEXIST) return -1;
    if (snprintf(log->path, sizeof log->path, "%s/sensord.csv", dir) >= (int)sizeof log->path) {
        errno = ENAMETOOLONG;
        return -1;
    }
    return open_current(log);
}

int csvlog_write(struct csvlog *log, const char *row) {
    if (log->rotate_lines > 0 && log->rows >= log->rotate_lines && rotate(log) != 0) return -1;
    char line[256];
    int n = snprintf(line, sizeof line, "%s\n", row);
    if (write_all(log->fd, line, (size_t)n) != 0) return -1;
    log->rows++;
    return 0;
}

void csvlog_close(struct csvlog *log) {
    if (log->fd >= 0) close(log->fd);
    log->fd = -1;
}
