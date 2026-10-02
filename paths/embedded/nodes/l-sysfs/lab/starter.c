#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Read a sysfs attribute as a string with trailing whitespace stripped.
 * Returns the length, or -errno (-ENOENT, -ERANGE if it doesn't fit in cap-1 bytes, ...). */
int sysfs_read_string(const char *path, char *buf, size_t cap) {
    (void)path;
    (void)buf;
    (void)cap;
    return -ENOSYS;
}

/* Read a sysfs attribute holding one decimal integer ("45123\n").
 * Returns 0 and sets *out, or -errno (-EINVAL for empty/garbage). */
int sysfs_read_long(const char *path, long *out) {
    (void)path;
    (void)out;
    return -ENOSYS;
}

/* Find the thermal zone under <root>/sys/class/thermal whose `type` equals `type`.
 * Returns the zone number and sets *temp_mc (millidegrees C), or -errno. */
int thermal_find_zone(const char *root, const char *type, long *temp_mc) {
    (void)root;
    (void)type;
    (void)temp_mc;
    return -ENOSYS;
}
