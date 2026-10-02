#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Read up to cap-1 bytes of a small attribute file into buf and NUL-terminate it.
 * Loops over partial reads; returns the byte count or -errno. -ERANGE if it doesn't fit. */
static int read_attr(const char *path, char *buf, size_t cap) {
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return -errno;
    size_t len = 0;
    int rc = 0;
    for (;;) {
        if (len == cap - 1) {               /* full: is there more? */
            char extra;
            ssize_t n = read(fd, &extra, 1);
            if (n > 0) rc = -ERANGE;
            else if (n < 0) rc = -errno;
            break;
        }
        ssize_t n = read(fd, buf + len, cap - 1 - len);
        if (n < 0) {
            if (errno == EINTR) continue;
            rc = -errno;
            break;
        }
        if (n == 0) break;                  /* end of file */
        len += (size_t)n;
    }
    close(fd);
    if (rc < 0) return rc;
    buf[len] = '\0';
    return (int)len;
}

int sysfs_read_string(const char *path, char *buf, size_t cap) {
    if (!path || !buf || cap == 0) return -EINVAL;
    int n = read_attr(path, buf, cap);
    if (n < 0) return n;
    size_t len = (size_t)n;
    while (len > 0 && isspace((unsigned char)buf[len - 1])) buf[--len] = '\0';
    return (int)len;
}

int sysfs_read_long(const char *path, long *out) {
    char buf[64];
    int n = sysfs_read_string(path, buf, sizeof buf);
    if (n < 0) return n;
    char *end;
    errno = 0;
    long v = strtol(buf, &end, 10);
    if (errno == ERANGE) return -ERANGE;
    if (end == buf || *end != '\0') return -EINVAL;   /* empty, or junk after the number */
    *out = v;
    return 0;
}

/* "thermal_zone<digits>" → the number, or -1 for anything else. */
static long zone_number(const char *name) {
    static const char prefix[] = "thermal_zone";
    if (strncmp(name, prefix, sizeof prefix - 1) != 0) return -1;
    const char *digits = name + sizeof prefix - 1;
    if (!isdigit((unsigned char)*digits)) return -1;
    char *end;
    long n = strtol(digits, &end, 10);
    return (*end == '\0' && n <= INT_MAX) ? n : -1;
}

int thermal_find_zone(const char *root, const char *type, long *temp_mc) {
    char path[PATH_MAX];
    if (snprintf(path, sizeof path, "%s/sys/class/thermal", root) >= (int)sizeof path) return -ENAMETOOLONG;
    DIR *dir = opendir(path);
    if (!dir) return -errno;

    long best = -1;                         /* readdir order is arbitrary: keep the lowest match */
    struct dirent *e;
    while ((e = readdir(dir)) != NULL) {
        long zone = zone_number(e->d_name);
        if (zone < 0 || (best >= 0 && zone >= best)) continue;
        char tpath[PATH_MAX], t[64];
        if (snprintf(tpath, sizeof tpath, "%s/%s/type", path, e->d_name) >= (int)sizeof tpath) continue;
        if (sysfs_read_string(tpath, t, sizeof t) >= 0 && strcmp(t, type) == 0) best = zone;
    }
    closedir(dir);
    if (best < 0) return -ENOENT;

    char tpath[PATH_MAX];
    if (snprintf(tpath, sizeof tpath, "%s/thermal_zone%ld/temp", path, best) >= (int)sizeof tpath) return -ENAMETOOLONG;
    int rc = sysfs_read_long(tpath, temp_mc);
    return rc < 0 ? rc : (int)best;
}
