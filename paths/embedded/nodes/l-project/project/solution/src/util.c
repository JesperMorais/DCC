#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
    return s;
}

int write_all(int fd, const char *buf, size_t len) {
    while (len > 0) {
        ssize_t n = write(fd, buf, len);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return -1;
        buf += n;
        len -= (size_t)n;
    }
    return 0;
}

int read_small(const char *path, char *buf, size_t len) {
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return -1;
    size_t got = 0;
    for (;;) {
        ssize_t n = read(fd, buf + got, len - 1 - got);
        if (n < 0 && errno == EINTR) continue;
        if (n < 0) {
            close(fd);
            return -1;
        }
        if (n == 0 || (got += (size_t)n) == len - 1) break;
    }
    close(fd);
    buf[got] = '\0';
    return 0;
}
