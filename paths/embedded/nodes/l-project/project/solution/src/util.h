#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stddef.h>

/* Strips leading and trailing whitespace (including \r and \n) in place. */
char *trim(char *s);

/* Writes all of buf, retrying on EINTR and short writes. 0 or -1. */
int write_all(int fd, const char *buf, size_t len);

/* Reads a small file (a sysfs attribute) into buf, NUL-terminated. 0 or -1. */
int read_small(const char *path, char *buf, size_t len);

#endif
